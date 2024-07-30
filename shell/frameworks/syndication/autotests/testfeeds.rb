#!/usr/bin/ruby

# This file is part of the syndication library
#
# SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>
#
# SPDX-License-Identifier: LGPL-2.0-or-later

SUBDIRS = [ 'rss2', 'rdf', 'atom' ]

TESTLIBSYNDICATION='../../build/syndication/tests/testlibsyndication'

numTotal = 0
numErrors = 0

files = Array.new

SUBDIRS.each do |dir|
    Dir.foreach(dir) do |i|
        files.push(dir + "/" + i) if i =~ /.*\.xml\Z/
    end
end

files.each do |file|
    expectedfn = file + ".expected"
    if File.exist?(expectedfn)
        expFile = File.open(expectedfn, "r")
        expected = expFile.read
        expFile.close
        
        system("#{TESTLIBSYNDICATION} #{file} > testfeeds-output.tmp")
        actFile = File.open("testfeeds-output.tmp")
        actual = actFile.read
        actFile.close
        
        numTotal += 1
        if actual != expected
            puts "#{file} parsed incorrectly (abstraction)."
            # TODO: add diff to log
            numErrors += 1
        end
              
    end
    specificfn = file + ".expected-specific"
    if File.exist?(specificfn)
        specFile = File.open(specificfn, "r")
        specific = specFile.read
        specFile.close
        
        system("#{TESTLIBSYNDICATION} --specific-format #{file} > testfeeds-output.tmp")
        actFile = File.open("testfeeds-output.tmp")
        actual = actFile.read
        actFile.close
        
        numTotal += 1
        if actual != specific
            puts "#{file} parsed incorrectly (specific format)."
            # TODO: add diff to log
            numErrors += 1
        end
              
    end

end
# TODO print more verbose output
exit(numErrors)

