def escapeURL(url)
    url2 = url.dup
    url2.gsub!('?', '_')
    url2.gsub!(':', '_')
    url2.gsub!('/', '_')
    url2.gsub!('=', '_')
    return url2.strip
end

lines = STDIN.readlines

lines.each do |i|
    system("wget \"#{i.strip}\" -O \"#{escapeURL(i)}\"")
end
