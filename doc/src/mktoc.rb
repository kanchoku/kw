#!/usr/local/bin/ruby
#
# usage: mkidx.rb *.rd > idx.rd

puts "=begin"
puts "=end"
puts "=begin html"
puts "<!-- mktoc: toc begin -->"	# do not change! (see mkframe.rb)
puts "<div class=\"toc\">"
puts "=end"
puts "=begin"
puts "‚à‚­‚¶"

while line = gets do
  line.chomp!
  next unless /^(=+)\s+(.*)$/ =~ line
  indent = ($1.length - 2) * 2
  indent = 0 if indent < 0
  str = $2
  puts (" " * indent + "* ((<" + str + ">))")
end

puts "=end"
puts "=begin html"
puts "</div>"
puts "<!-- mktoc: toc end -->"		# do not change! (see mkframe.rb)
puts "=end"
puts "=begin"
puts "=end"
