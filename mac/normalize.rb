#!/usr/bin/env ruby -KU

require 'rubygems'
require 'activesupport'

print STDIN.readlines.map { |i| i.chars.normalize }

