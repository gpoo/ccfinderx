#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright: This module has been placed in the public domain.

"""
This module declares required methods of CCFinderX preprocessor.
"""

def to_version_str(intTuple):
	return "_".join([ str(i) for i in intTuple])

def from_version_str(versionStr):
	return tuple([ int(s) for s in versionStr.split("_")])

class Param(object):
	ANY_MATCH = 1
	EXACT_MATCH = 2
	P_MATCH = 3

class Base(object):
	def tonormalizedoptionstring(self, optionStr):
		if optionStr not in ( None, '', 'default' ):
			raise preprocessor.InvalidOptionError, "invalid option: " + optionStr
		return 'default'
	
	def getoptiondescription(self):
		return "description of options"
	
	def setoptions(self, optionStr): # may throw InvalidOptionError
		pass
	
	def getnormalizedoptionstring(self):
		return 'default'
	
	def getdefaultparameterizing(self):
		return dict()
	
	def parse(self, sourceCodeStrInUtf8):
		return ''
	
class InvalidOptionError(ValueError):
	pass


