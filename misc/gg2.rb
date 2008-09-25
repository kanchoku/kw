#!/usr/local/bin/ruby -Ks

# From: [211] 99 <sage>
# Date: 03/09/28 02:33
# 
# ちなみに辞書のフォーマットを拡張しました。
# 
# 辞書作成スクリプトの使い方
# ・1単語ごとに改行してあるファイルを作る。
# （kwmaze.dicからなら行頭のキーを消した後"/"を改行に置換するとか）
# ・下の2つのファイルをgg1.rb、gg2.rbとして保存。
# ・後は
# ruby -Ks gg1.rb 元ファイル | sort | uniq | ruby -Ks gg2.rb > 辞書ファイル
# 
# ……perlじゃなくてごめんなさい。
# 
# From: [212] 99 <sage>
# Date: 03/09/28 02:33
# 
# gg2.rb
# --------

key = ""
while gets
  chop!
  if key.length > 0 && key.length+2 == $_.length && key == $_[0,key.length]
    print $_[-2..-1]
  else
    if key.length > 0
      print "\r\n"
    end
    key = $_.chop
    key.chop!
    print key
    print $_[-2..-1]
  end
end
print "\r\n"

# --------
