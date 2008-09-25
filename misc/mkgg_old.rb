#!/usr/local/bin/ruby -Ks
#
# mkgg.rb - 熟語ガイド辞書 (旧形式) を作成
#
# usage: ruby mkgg.rb kwmaze.dic > gg.dic
#        ruby mkgg.rb general_text_file > gg.dic
#
# ■注意■
#
# これは、旧形式の熟語ガイド辞書、すなわち、
#   『【漢直】T-Code/TUT-Codeスレ Lesson2【連習】』
#   <http://pc.2ch.net/test/read.cgi/unix/1061936143/>
# の 102-106 氏のコードの熟語ガイド機能用の辞書を作成するスクリプトです。
#
# 漢字どうしの連なり、二文字連鎖のみを、処理対象としています。
# かなや「〆々〇」以外の記号類は無視されます。
#
# 新版の熟語ガイド機能 (上記スレの 208-212 氏のコード) でも
# 旧形式の辞書を利用することは、可能なようですが、たぶん非推奨です。
#
# 2003-09-28
#	* first ver.

KanjiPat = /[〆々〇亜-腕弌-熙]/

$gghash = Hash::new

# --------------------------------------------------------------------
# methods

def add2gg(leader, trailer)
  if $gghash[leader].nil? then
    $gghash[leader] = Hash::new
  end
  if ($gghash[leader])[trailer].nil? then
    ($gghash[leader])[trailer] = 0
  end
  ($gghash[leader])[trailer] += 1
end

def outputgg
  $gghash.sort.each do |leader, trailers|
    t = trailers.to_a.sort { |va, vb|
      va[1] <=> vb[1]
    } .reverse.collect { |v|
      v[0]
    }
    puts "#{leader}#{t.join}"
  end
end

# --------------------------------------------------------------------
# main

while line = gets
  line.chomp!

  ary = line.split(//)
  while 2 <= ary.length
    if KanjiPat =~ ary[0] && KanjiPat =~ ary[1] then
      add2gg(ary[0], ary[1])
    end
    ary.shift
  end
end

outputgg
