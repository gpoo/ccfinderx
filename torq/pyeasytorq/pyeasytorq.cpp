// pyeasytorq.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include <cassert>
#include <cstring> // strlen

#include <vector>

#include <Python.h>

#include <boost/static_assert.hpp>

#include <unicode/uversion.h>

#include "../easytorq/easytorq.h"
#include "../../common/utf8support.h"

#if defined _MSC_VER

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#endif

/* Tree */

typedef struct {
    PyObject_HEAD
	easytorq::Tree *pTree;
} Tree;

static void
Tree_dealloc(Tree *self)
{
	assert(self != NULL);

	if (self->pTree != NULL) {
		delete self->pTree;
	}

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Tree_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Tree *self = (Tree *)type->tp_alloc(type, 0);
    if (self != NULL) {
		self->pTree = NULL;
    }

    return (PyObject *)self;
}

static int
Tree_init(Tree *self, PyObject *args, PyObject *kwds)
{
	assert(self != NULL);

	const char *str = NULL;
	if (! PyArg_ParseTuple(args, "s", &str)) {
		return -1;
	}

	self->pTree = new easytorq::Tree(str);

    return 0;
}

static PyMethodDef Tree_methods[] = {
    { NULL }  /* Sentinel */
};

static PyTypeObject TreeType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "easytorq.Tree",             /*tp_name*/
    sizeof(Tree),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Tree_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "easytorq Tree objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Tree_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Tree_init,      /* tp_init */
    0,                         /* tp_alloc */
    Tree_new,                 /* tp_new */
};

/* Pattern */

typedef struct {
    PyObject_HEAD
	easytorq::Pattern *pPattern;
} Pattern;

static void
Pattern_dealloc(Pattern* self)
{
	assert(self != NULL);

	if (self->pPattern != NULL) {
		delete self->pPattern;
	}

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Pattern_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Pattern *self = (Pattern *)type->tp_alloc(type, 0);
    if (self != NULL) {
		self->pPattern = NULL;
    }

    return (PyObject *)self;
}

static int
Pattern_init(Pattern *self, PyObject *args, PyObject *kwds)
{
	assert(self != NULL);

	const char *str = NULL;
	if (! PyArg_ParseTuple(args, "s", &str)) {
		return -1;
	}

	try {
		self->pPattern = new easytorq::Pattern(str);
	}
	catch (easytorq::ParseError &e) {
		PyErr_SetString(PyExc_ValueError, "invalid pattern string.");
		return -1;
	}

    return 0;
}

static PyObject *
Pattern_setcutoffvalue(Pattern *self, PyObject *args)
{
	assert(self != NULL);

	long value;
	if (! PyArg_ParseTuple(args, "l", &value)) {
		return NULL;
	}

	if (self->pPattern != NULL) {
		self->pPattern->setCutoffValue(value);
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
Pattern_apply(Pattern *self, PyObject *args)
{
	assert(self != NULL);

	Tree *pTree = NULL;
	if (! PyArg_ParseTuple(args, "O!", &TreeType, &pTree)) {
		return NULL;
	}

	if (self->pPattern != NULL) {
		if (pTree->pTree != NULL) {
			try {
				self->pPattern->apply(pTree->pTree);
			}
			catch (easytorq::InterpretationError &e) {
				PyErr_SetString(PyExc_ValueError, "interpretation error.");
				return NULL;
			}
		}
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef Pattern_methods[] = {
	{ "setcutoffvalue", (PyCFunction)Pattern_setcutoffvalue, METH_VARARGS, "set cutoff value to pattern." },
	{ "apply", (PyCFunction)Pattern_apply, METH_VARARGS, "apply the pattern to an argument tree." },
    { NULL }  /* Sentinel */
};

static PyTypeObject PatternType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "easytorq.Pattern",             /*tp_name*/
    sizeof(Pattern),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Pattern_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "easytorq Pattern objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Pattern_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Pattern_init,      /* tp_init */
    0,                         /* tp_alloc */
    Pattern_new,                 /* tp_new */
};

/* CngFormatter */

typedef struct {
    PyObject_HEAD
	easytorq::CngFormatter *pCngFormatter;
} CngFormatter;

static void
CngFormatter_dealloc(CngFormatter* self)
{
	assert(self != NULL);

	if (self->pCngFormatter != NULL) {
		delete self->pCngFormatter;
	}

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
CngFormatter_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CngFormatter *self = (CngFormatter *)type->tp_alloc(type, 0);
    if (self != NULL) {
		self->pCngFormatter = NULL;
    }

    return (PyObject *)self;
}

static int
CngFormatter_init(CngFormatter *self, PyObject *args, PyObject *kwds)
{
	assert(self != NULL);

	if (! PyArg_ParseTuple(args, "")) {
		return -1;
	}

	self->pCngFormatter = new easytorq::CngFormatter();

    return 0;
}

static PyObject *
CngFormatter_addNodeFlatten(CngFormatter *self, PyObject *args)
{
	assert(self != NULL);

	const char *str;
	if (! PyArg_ParseTuple(args, "s", &str)) {
		return NULL;
	}

	if (self->pCngFormatter != NULL) {
		self->pCngFormatter->addNodeFlatten(str);
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
CngFormatter_addNodeNone(CngFormatter *self, PyObject *args)
{
	assert(self != NULL);

	const char *str;
	if (! PyArg_ParseTuple(args, "s", &str)) {
		return NULL;
	}

	if (self->pCngFormatter != NULL) {
		self->pCngFormatter->addNodeNone(str);
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
CngFormatter_addNodeTerminate(CngFormatter *self, PyObject *args)
{
	assert(self != NULL);

	const char *str;
	if (! PyArg_ParseTuple(args, "s", &str)) {
		return NULL;
	}

	if (self->pCngFormatter != NULL) {
		self->pCngFormatter->addNodeTerminate(str);
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
CngFormatter_addNodeReplace(CngFormatter *self, PyObject *args)
{
	assert(self != NULL);

	const char *strNodeName;
	const char *strNewName;
	if (! PyArg_ParseTuple(args, "ss", &strNodeName, &strNewName)) {
		return NULL;
	}

	if (self->pCngFormatter != NULL) {
		self->pCngFormatter->addNodeReplace(strNodeName, strNewName);
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
CngFormatter_addNodeFormat(CngFormatter *self, PyObject *args)
{
	assert(self != NULL);

	const char *strNodeName;
	const char *strOpenStr;
	const char *strCloseStr;
	if (! PyArg_ParseTuple(args, "sss", &strNodeName, &strOpenStr, &strCloseStr)) {
		return NULL;
	}

	if (self->pCngFormatter != NULL) {
		self->pCngFormatter->addNodeFormat(strNodeName, strOpenStr, strCloseStr);
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
CngFormatter_format(CngFormatter *self, PyObject *args)
{
	assert(self != NULL);

	Tree *pTree = NULL;
	if (! PyArg_ParseTuple(args, "O!", &TreeType, &pTree)) {
		return NULL;
	}

	if (self->pCngFormatter != NULL) {
		if (pTree->pTree != NULL) {
			std::string str = self->pCngFormatter->format(*pTree->pTree);
			PyObject *value = PyString_FromStringAndSize(str.data(), str.length());
			return value;
		}
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef CngFormatter_methods[] = {
	{ "addflatten", (PyCFunction)CngFormatter_addNodeFlatten, METH_VARARGS, "set node format flat." },
	{ "addnone", (PyCFunction)CngFormatter_addNodeNone, METH_VARARGS, "set node format none." },
	{ "addterminate", (PyCFunction)CngFormatter_addNodeTerminate, METH_VARARGS, "set node format terminate." },
	{ "addreplace", (PyCFunction)CngFormatter_addNodeReplace, METH_VARARGS, "set node format replace." },
	{ "addformat", (PyCFunction)CngFormatter_addNodeFormat, METH_VARARGS, "set node format." },
	{ "format", (PyCFunction)CngFormatter_format, METH_VARARGS, "format an argument tree." },
    { NULL }  /* Sentinel */
};

static PyTypeObject CngFormatterType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "easytorq.CngFormatter",             /*tp_name*/
    sizeof(CngFormatter),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)CngFormatter_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "easytorq CngFormatter objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    CngFormatter_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)CngFormatter_init,      /* tp_init */
    0,                         /* tp_alloc */
    CngFormatter_new,                 /* tp_new */
};

/* ICUConverter */

typedef struct {
    PyObject_HEAD
	Decoder *pDecoder;
} ICUConverter;

static void
ICUConverter_dealloc(ICUConverter *self)
{
	assert(self != NULL);

	if (self->pDecoder != NULL) {
		delete self->pDecoder;
	}

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
ICUConverter_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    ICUConverter *self = (ICUConverter *)type->tp_alloc(type, 0);
    if (self != NULL) {
		self->pDecoder = NULL;
    }

    return (PyObject *)self;
}

static int
ICUConverter_init(ICUConverter *self, PyObject *args, PyObject *kwds)
{
	assert(self != NULL);

	if (! PyArg_ParseTuple(args, "")) {
		return -1;
	}

	self->pDecoder = new Decoder();

    return 0;
}

static PyObject *
ICUConverter_setEncoding(ICUConverter *self, PyObject *args)
{
	assert(self != NULL);

	const char *str;
	if (! PyArg_ParseTuple(args, "s", &str)) {
		return NULL;
	}

	if (self->pDecoder != NULL) {
		if (! self->pDecoder->setEncoding(str)) {
			PyErr_SetString(PyExc_ValueError, "invalid encoding name.");
			return NULL;
		}
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ICUConverter_getEncoding(ICUConverter *self, PyObject *args)
{
	assert(self != NULL);

	if (! PyArg_ParseTuple(args, "")) {
		return NULL;
	}

	if (self->pDecoder != NULL) {
		std::string str = self->pDecoder->getEncoding();
		return PyString_FromStringAndSize(str.data(), str.length());
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ICUConverter_getAvailableEncodings(ICUConverter *self, PyObject *args)
{
	assert(self != NULL);

	if (! PyArg_ParseTuple(args, "")) {
		return NULL;
	}

	if (self->pDecoder != NULL) {
		std::vector<std::string> encodings = self->pDecoder->getAvailableEncodings();
		PyObject *pList = PyList_New(0);
		if (pList == NULL) {
			PyErr_SetString(PyExc_ValueError, "internal error: fail to create a list.");
			return NULL;
		}
		for (size_t i = 0; i < encodings.size(); ++i) {
			const std::string &enci = encodings[i];
			PyObject *pStr = PyString_FromStringAndSize(enci.data(), enci.length());
			if (PyList_Append(pList, pStr) != 0) {
				PyErr_SetString(PyExc_ValueError, "internal error: fail to insert an item to a list.");
				Py_XDECREF(pList);
				return NULL;
			}
		}
		return pList;
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ICUConverter_getICUVersion(ICUConverter *self, PyObject *args)
{
	assert(self != NULL);

	if (! PyArg_ParseTuple(args, "")) {
		return NULL;
	}

	if (self->pDecoder != NULL) {
		// typedef uint8_t UVersionInfo[U_MAX_VERSION_LENGTH];
		uint8_t versionInfo[U_MAX_VERSION_LENGTH];
		BOOST_STATIC_ASSERT(U_MAX_VERSION_LENGTH == 4);

		u_getVersion(versionInfo);
		return Py_BuildValue("iiii", (int)versionInfo[0], (int)versionInfo[1], (int)versionInfo[2], (int)versionInfo[3]);
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ICUConverter_decode(ICUConverter *self, PyObject *args)
{
	assert(self != NULL);

	const char *str;
	if (! PyArg_ParseTuple(args, "s", &str)) {
		return NULL;
	}

	if (self->pDecoder != NULL) {
		std::vector<MYWCHAR_T> vec;
		size_t strLength = strlen(str);
		self->pDecoder->decode(&vec, &str[0], &str[strLength]);
		std::string str = toUTF8String(vec);
		return PyString_FromStringAndSize(str.data(), str.length());
	}

	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ICUConverter_encode(ICUConverter *self, PyObject *args)
{
	assert(self != NULL);

	const char *str;
	if (! PyArg_ParseTuple(args, "s", &str)) {
		return NULL;
	}

	if (self->pDecoder != NULL) {
		std::vector<MYWCHAR_T> vec;
		toWStringV(&vec, str);
		std::string str = self->pDecoder->encode(vec);
		return PyString_FromStringAndSize(str.data(), str.length());
	}
	
	// return None
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef ICUConverter_methods[] = {
	{ "setencoding", (PyCFunction)ICUConverter_setEncoding, METH_VARARGS, "set character encoding." },
	{ "getencoding", (PyCFunction)ICUConverter_getEncoding, METH_VARARGS, "get character encoding." },
	{ "getavailableencodings", (PyCFunction)ICUConverter_getAvailableEncodings, METH_VARARGS, "get a list of available character encodings." },
	{ "geticuversion", (PyCFunction)ICUConverter_getICUVersion, METH_VARARGS, "get version of ICU." },
	{ "decode", (PyCFunction)ICUConverter_decode, METH_VARARGS, "convert from encoded string to utf8 string." },
	{ "encode", (PyCFunction)ICUConverter_encode, METH_VARARGS, "convert from utf8 string to encoded string." },
    { NULL }  /* Sentinel */
};

static PyTypeObject ICUConverterType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "easytorq.ICUConverter",             /*tp_name*/
    sizeof(ICUConverter),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)ICUConverter_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "easytorq ICUConverter objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    ICUConverter_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)ICUConverter_init,      /* tp_init */
    0,                         /* tp_alloc */
    ICUConverter_new,                 /* tp_new */
};


/* module methods */

static PyObject *
easytorq_version(PyObject *self, PyObject *args)
{
	if (! PyArg_ParseTuple(args, "")) {
		return NULL;
	}
	return Py_BuildValue("(iiii)", 10, 2, 2, 0);
}

static PyObject *
easytorq_credits(PyObject *self, PyObject *args)
{
	static const char *str = "easytorq (C) 2009-2010 AIST";

	if (! PyArg_ParseTuple(args, "")) {
		return NULL;
	}
	return PyString_FromString(str);
}

static PyMethodDef module_methods[] = {
	{ "version", easytorq_version, METH_VARARGS, "get a tuple of version number." },
	{ "credits", easytorq_credits, METH_VARARGS, "get a credit." },
    { NULL, NULL, 0, NULL }  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initeasytorq(void) 
{
    PyObject* m;

    if (PyType_Ready(&TreeType) < 0)
        return;

	if (PyType_Ready(&PatternType) < 0)
		return;

	if (PyType_Ready(&CngFormatterType) < 0)
		return;

	if (PyType_Ready(&ICUConverterType) < 0)
		return;

    m = Py_InitModule3("easytorq", module_methods,
                       "easytorq module.");

    if (m == NULL)
      return;

    Py_INCREF(&TreeType);
    PyModule_AddObject(m, "Tree", (PyObject *)&TreeType);

	Py_INCREF(&PatternType);
	PyModule_AddObject(m, "Pattern", (PyObject *)&PatternType);

	Py_INCREF(&CngFormatterType);
	PyModule_AddObject(m, "CngFormatter", (PyObject *)&CngFormatterType);

	Py_INCREF(&ICUConverterType);
	PyModule_AddObject(m, "ICUConverter", (PyObject *)&ICUConverterType);
}
