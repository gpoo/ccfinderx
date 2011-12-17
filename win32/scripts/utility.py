#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright: This module has been placed in the public domain.

import sys

class ProgressReporter(object):
	def __init__(self, totalAmount):
		if totalAmount == 0:
			self.silence = True
			return
		self.silence = False
		self.totalAmount = totalAmount
		self.currentValue = 0
		self.resolution = 80
		self.outp = sys.stderr
	
	def proceed(self, currentValue):
		if self.silence: return
		if currentValue > self.currentValue:
			lastTick = self.currentValue * self.resolution // self.totalAmount
			curTick = currentValue * self.resolution // self.totalAmount
			for i in range(lastTick, curTick):
				if i % (self.resolution/5) == 0:
					self.outp.write("%d%%" % (100 * i // self.resolution))
				if i % 2 == 0:
					s = '.'
				else:
					s = '\b:'
				self.outp.write(s)
				self.outp.flush()
		self.currentValue = currentValue
	
	def done(self):
		if self.silence: return
		if self.totalAmount == 0:
			return
		self.proceed(self.totalAmount)
		curTick = self.currentValue * self.resolution // self.totalAmount
		if curTick % (self.resolution/5) == 0:
			self.outp.write("%d%%" % (100 * curTick // self.resolution))
		self.outp.write("\n")
		self.outp.flush()
	
	def abort(self):
		if self.silence: return
		if self.totalAmount == 0:
			return
		self.outp.write("\n")
		self.outp.flush()

def escapeCommandline(args):
    r = list()
    for a in args[:]:
        if ' ' in a:
            r.append('"%s"' % a)
        else:
            r.append(a)
    return r

