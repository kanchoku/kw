#!/usr/local/bin/ruby
#
# usage: 
#   mkframe.rb kanchoku.html : () -> ()
#   :- kanchoku_frame.html, kanchoku_frame_toc.html, kanchoku_frame_content.html

# --------------------------------------------------------------------

if ARGV.length != 1 then
  $stderr.puts "usage: #{$0} kanchoku.html"
  exit 2
end

# --------------------------------------------------------------------

$srcfile = ARGV.shift			# "kanchoku.html"
$basename = $srcfile.sub(/\.html/, "")	# "kanchoku"

$framefile = $basename + "_frame.html"
$tocfile = $basename + "_frame_toc.html"
$contentfile = $basename + "_frame_content.html"

$tocname = "toc"
$contentname = "content"

# --------------------------------------------------------------------

open($srcfile) do |fsrc|
  open($tocfile, "w") do |ftoc|
    open($contentfile, "w") do |fcontent|
      outs = :both
      while line = fsrc.gets do
        line.chomp!
        case line
        when /<h1>/i
          outs = :content
        when /<!-- mktoc: toc begin -->/i
          outs = :toc
        when /<\/body>/i
          outs = :both
        end # case line

        if outs == :content || outs == :both then
          fcontent.puts line.gsub(/(<a) (href="http:\/\/)/i) {
            "#{$1} target=\"_top\" #{$2}"
          }
        end

        if outs == :toc || outs == :both then
          line.sub!(/<div class="toc">/i, '<div class="toc_frame">')
          line.gsub!(/(<a) (href=")#/i) {
            "#{$1} target=\"#{$contentname}\" #{$2}#{$contentfile}#" 
          }
          ftoc.puts line
        end

        case line
        when /<!-- mktoc: toc end -->/i
          outs = :content
        end # case line
      end # while gets
    end # fcontent
  end # ftoc
end # fsrc

# --------------------------------------------------------------------

open($framefile, "w") do |fframe|
  fframe.puts <<"EOH"
<?xml version="1.0" encoding="Shift_JIS"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Frameset//EN"
 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ja" lang="ja">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=Shift_JIS" />
<title>kw127</title>
<link href="style/kanchoku.css" type="text/css" rel="stylesheet" />
</head>
<frameset cols="240,*" title="kw127">
 <frame src="#{$tocfile}" title="#{$tocname}" id="#{$tocname}" name="#{$tocname}" />
 <frame src="#{$contentfile}" title="#{$contentname}" id="#{$contentname}" name="#{$contentname}" />
 <noframes>
  <body>
  <a href="#{$srcfile}">#{$srcfile} ÇÇ≤óóÇ≠ÇæÇ≥Ç¢ÅB</a>
  </body>
 </noframes>
</frameset>
</html>
EOH
end # fframe
