#!/usr/local/bin/ruby -i~ -p
#
# usage: postproc.rb RD_OUTPUT_HTML

=begin
== RD では満足できない人のための、タグ追加 dirty trick

* [^hoge^]  → <span class="sup">hoge</span>
* [{fuga}hoge] → <span class="fuga">hoge</span>
* [:tag:] → <tag>
=end

$_.gsub!(/\[\^(.*?)\^\]/) { "<span class=\"sup\">#{$1}</span>" }
$_.gsub!(/\[\{(.*?)\}(.*?)\]/) { "<span class=\"#{$1}\">#{$2}</span>" }
$_.gsub!(/\[:(.*?):\]/) { "<#{$1}>" }
