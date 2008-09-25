#!/usr/local/bin/ruby -Ks
#
# reducegg.rb - 覚えた漢字を、熟語ガイド辞書から削除
#
# usage: ruby reducegg.rb CERTAIN kwgg.dic > mygg.dic
#
# 2003-09-29
#	* 熟語ガイド辞書の新フォーマット (『連習スレ 2』208-212) に対応
#	* 特に、候補が空になったエントリは削除
# 2003-09-28
# 	* fisrt ver.

# 覚えた漢字のリスト
$certains = Array::new;

if ARGV.length <= 0 then
  $stderr.puts "usage: #{$0} CERTAIN [gg_dictionary]"
  exit 2
end

# CERTAIN を読みこむ
open(ARGV.shift) do |f|
  while line = f.gets do
    line.strip.split(//).each do |k|
      $certains.push(k) if /\s/ !~ k
    end
  end
end
$certains = $certains.sort.uniq

# 熟語ガイド辞書を読み込み、加工しながら、出力する
while line = gets do
  line.chomp!
  case line
  when /^([^ \/]+) \/(.*)$/
    # new format GG-dic
    leader, trailers = $1, $2.split(//)
    trailers.delete_if do |k|
      $certains.include?(k)
    end
    puts(leader + " /" + trailers.join("")) if 0 < trailers.length

  when /^(.)(.*)$/
    # old format GG-dic
    leader, trailers = $1, $2.split(//)
    trailers.delete_if do |k|
      $certains.include?(k)
    end
    puts(leader + trailers.join("")) if 0 < trailers.length

  else
    # invalid line
    $stderr.puts " skipped: #{line}"
  end # case line
end
