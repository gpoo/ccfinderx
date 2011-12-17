// taken form http://yusuke.rurinet.ne.jp/wiki/wiki.cgi?page=STL+streambuf

#ifndef __WIN32_STREAM_HANDLE_HEADER__INCLUDED__
#define __WIN32_STREAM_HANDLE_HEADER__INCLUDED__

#include <windows.h>

#include <iostream>
#include <vector>
#include <stdexcept>

namespace win32 {

class Win32Exception : public std::runtime_error {
public:
	DWORD errorCode;
public:
	Win32Exception(DWORD errorCode_)
		: std::runtime_error("Win32Exception"), errorCode(errorCode_)
	{
	}
};

template <class Ch,class Tr=std::char_traits<Ch> >
class basic_win32handle_streambuf : public std::basic_streambuf<Ch,Tr> {
public:
	explicit basic_win32handle_streambuf(HANDLE h) : handle(h), bufferSize(1024) { init(); }
	basic_win32handle_streambuf(HANDLE h, int buffer) : handle(h), bufferSize(buffer) { init(); }
	virtual ~basic_win32handle_streambuf();

protected:
	const int bufferSize;
	void init();
	HANDLE handle;

};

template <class Ch,class Tr=std::char_traits<Ch> >
class basic_win32handle_ostreambuf : public basic_win32handle_streambuf<Ch,Tr> {
public:
	explicit basic_win32handle_ostreambuf(HANDLE h) : basic_win32handle_streambuf<Ch,Tr>(h) { init(); }
	basic_win32handle_ostreambuf(HANDLE h, int buffer) : basic_win32handle_streambuf<Ch,Tr>(h, buffer) { init(); }
	virtual ~basic_win32handle_ostreambuf();

protected:
	virtual int_type overflow(int_type c=Tr::eof())
	{
		// バッファがあふれたので書き込みを行う
		write();

		// 下はお決まりらしい。よく知らないｗ
		if(c!=Tr::eof()){
			// 受け取った文字をバッファに挿入
			*pptr()=Tr::to_char_type(c);
			// バッファの書き込み位置をインクリメント
			pbump(1);
			return Tr::not_eof(c);

		} else {
			return Tr::eof();
		}
	}

	virtual int_type sync()
	{
		flush();
		return 0;
	}

private:
	Ch* writeBuffer;
	void write();
	void flush();
	void init();

};


template <class Ch,class Tr=std::char_traits<Ch> >
class basic_win32handle_istreambuf : public basic_win32handle_streambuf<Ch,Tr> {
public:
	explicit basic_win32handle_istreambuf(HANDLE h) : basic_win32handle_streambuf<Ch,Tr>(h) { init(); }
	basic_win32handle_istreambuf(HANDLE h, int buffer) : basic_win32handle_streambuf<Ch,Tr>(h, buffer) { init(); }

	virtual ~basic_win32handle_istreambuf();

protected:
	virtual int_type underflow(void)
	{
		if(egptr()<=gptr()){
			// バッファを使い果たしたので読み込みを行う
			read();

			if(egptr()<=gptr()){
				// 結局1バイトも読めなかった
				return Tr::eof();
			}
		}

		// 読めたので最初の一個を返す
		return Tr::to_int_type(*gptr());
	}


private:
	Ch* readBuffer;
	void read();
	void init();

};



/**
 * Win32 HANDLE 版 iostream
 */
template <class Ch,class Tr=std::char_traits<Ch> >
class basic_win32handle_istream : public std::basic_istream<Ch,Tr> {
public:
	/**
	 * HANDLE から iostream を構築する
	 */
	basic_win32handle_istream(HANDLE h)
		: std::basic_istream<Ch,Tr>(new basic_win32handle_istreambuf<Ch,Tr>(h))
	{
	}

	~basic_win32handle_istream(void)
	{
		delete rdbuf();
	}

private:

};

/**
 * Win32 HANDLE 版 iostream
 */
template <class Ch,class Tr=std::char_traits<Ch> >
class basic_win32handle_ostream : public std::basic_ostream<Ch,Tr> {
public:
	/**
	 * HANDLE から iostream を構築する
	 */
	basic_win32handle_ostream(HANDLE h)
		: std::basic_ostream<Ch,Tr>(new basic_win32handle_ostreambuf<Ch,Tr>(h))
	{
	}

	~basic_win32handle_ostream(void)
	{
		delete rdbuf();
	}

private:

};


template <class Ch,class Tr>
inline
void basic_win32handle_ostreambuf<Ch, Tr>::init()
{
	writeBuffer = new Ch[bufferSize];

	// 書き込みポインタの設定
	// バッファ終端までかけるように
	setp( writeBuffer, writeBuffer + bufferSize );

}

template <class Ch,class Tr>
inline
void basic_win32handle_istreambuf<Ch, Tr>::init()
{
	readBuffer  = new Ch[bufferSize];

	// 読み取りポインタの設定
	// 最初は何も読み込んでいない状態
	setg( readBuffer, readBuffer + bufferSize, readBuffer + bufferSize );
}

/**
 * basic_win32handle_streambufの初期化を行う
 */
template <class Ch,class Tr>
inline
void basic_win32handle_streambuf<Ch, Tr>::init()
{
}

template <class Ch,class Tr>
inline
basic_win32handle_ostreambuf<Ch, Tr>::~basic_win32handle_ostreambuf()
{
	flush();
	delete [] writeBuffer;

}

template <class Ch,class Tr>
inline
basic_win32handle_istreambuf<Ch, Tr>::~basic_win32handle_istreambuf()
{
	delete [] readBuffer;
}

template <class Ch,class Tr>
inline
basic_win32handle_streambuf<Ch, Tr>::~basic_win32handle_streambuf()
{
}

/**
 * 実際にハンドルを使ってバッファを書き出す
 */
template <class Ch,class Tr>
inline
void basic_win32handle_ostreambuf<Ch, Tr>::write()
{
	DWORD ws = (pptr() - pbase()) * sizeof(Ch);
	DWORD written;

	BOOL rc = WriteFile(handle, pbase(), ws, &written, NULL);
	if (FALSE == rc)
		throw Win32Exception(::GetLastError());

	setp(pbase(), epptr());
}

/**
 * 実際にハンドルを使ってバッファに読み込む
 */
template <class Ch,class Tr>
inline
void basic_win32handle_istreambuf<Ch, Tr>::read()
{
	DWORD rs = bufferSize * sizeof(Ch);
	DWORD readed;

	BOOL rc = ReadFile(handle, readBuffer, rs, &readed, NULL);
	if (FALSE == rc)
		throw Win32Exception(::GetLastError());

	setg(readBuffer, readBuffer, readBuffer + readed);
}

/**
 * バッファをフラッシュする
 */
template <class Ch,class Tr>
inline
void basic_win32handle_ostreambuf<Ch, Tr>::flush()
{
	// flush する
	write();
}

}; // namespace win32


#endif //__WIN32_STREAM_HANDLE_HEADER__INCLUDED__

