opml = STDIN.readlines.join

opml.scan(/xmlUrl="([^"]*)"/) do |match|
    m2 = match[0].gsub('&amp;', '&')
    puts m2
end

