=begin

== はじめに

=== 「漢直Win」とは

「漢直Win」は、Windows 上で((*漢字直接入力*))を行うためのツールです。
((<徳岡宏樹|URL:http://t.pos.to/tc/>)) さんによって作製され、
現在 T-Code メーリングリストなどにおいて改良が加えられています。

このドキュメントでは、「漢直Win」バージョン 1.27 について解説します。

「漢直Win 1.27」の主な特徴は、次のとおりです。

* T-Code, TUT-Code, G-Code が使えます。
* ((<部首合成変換>))、((<交ぜ書き変換>))、((<ヒストリ入力>))
  などの補助入力が使えます。再帰的に組み合わせることも可能です。
* 補助入力した文字のコードを表示する((<文字ヘルプ>))が利用できます。
* ((<熟語ガイド>))の入力補助が利用できます。[^[((<ver1.27a|変更履歴>))]^]
* ((<入力の統計>))を記録することができます。[^[((<ver1.27f|変更履歴>))]^]

なお、ここでは漢字直接入力そのものについては触れません。
漢字直接入力については、((<関連 URL>)) を参照してください。

=== 動作環境

Windows95 以降の Windows で動作するものと思われます。

=== 制限事項

「漢直Win」は、まっとうな IME ではなく、
WM_CHAR などのメッセージを送りつけることで文字を入力する
という方法をとっています。
したがって、このようなメッセージに対応していないソフトでは、
「漢直Win」を利用することはできません。

たとえば、コマンドプロンプト (MS-DOS プロンプト) では、
入力を行うことができません。

特に、Windows 95/98/Me などの MS-DOS プロンプトでは、
「漢直Win」で入力しようとすると、
コンピュータがしばらく入力を受けつけなくなることがあります。
((*((*MS-DOS プロンプトを使用する時は、
必ず「漢直Win」を OFF に してください。*))*))
MS-DOS プロンプトでは、
「漢直Win」のトグルキー (((%Ctrl%))+((%\%))) も使えませんので、
「漢直Win」のウィンドウをクリックして OFF にしてください。

=== 使用条件

このソフトは((*無償*))、((*無保証*))です。

「漢直Win」バージョン 1.27 の原型である、
オリジナル版の「漢直Win」バージョン 1.26
(((<徳岡宏樹|URL:http://t.pos.to/tc/>)) さんの web ページより入手可能の 
kanwin126.zip) の使用条件は、
同パッケージに含まれるドキュメント kanchoku.txt によると、
以下のようなものです。

kanchoku.txt
  ●ソフトの使用条件
    このソフトは使用も配布も自由ですが、このソフトを使用したことによって
  何か問題が発生しても、作者も関係者もなんら責任を負いません。自己責任で
  使用してください。

一方、「漢直Win」バージョン 1.27 の部首合成変換のコード 
(bushu_dic.c) は、
((<"tserv-0.2"|URL:http://www.tcp-ip.or.jp/~tagawa/archive/>)) の comp.c 
に基づくものであり、
さらにこの comp.c は、
((<"GNU GPL"|URL:http://www.gnu.org/licenses/gpl.html>))
ライセンスの tc.el を原型としているとあります。

bushu_dic.c
  /*
    このプログラムの原型は, tc.el から作られました.  tc.el の Copyright
    は以下の通りです.
  ;;
  ;; T-Code frontend for Nemacs.
  ;; Author : Yasushi Saito (yasushi@is.s.u-tokyo.ac.jp)
  ;;          Kaoru Maeda   (kaoru@is.s.u-tokyo.ac.jp)
  ;;
  ;; Department of Information Science
  ;; Faculty of Science, University of Tokyo
  ;; 7-3-1 Hongo, Bunkyo-ku, 113 Tokyo, Japan
  ;;
  ;; Copyright (C) 1989, 1990, 1991 Yasushi Saito and Kaoru Maeda.
  ;;
  
  ;; This program is free software; you can redistribute it and/or modify
  ;; it under the terms of the GNU General Public License version 1
  ;; as published by the Free Software Foundation.
  ;;
  ;; This program is distributed in the hope that it will be useful,
  ;; but WITHOUT ANY WARRANTY; without even the implied warranty of
  ;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  ;; GNU General Public License for more details.
  ;;
  ;; You should have received a copy of the GNU General Public License
  ;; along with this program; if not, write to the Free Software
  ;; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  */

以上のことから、結果として、「漢直Win」1.27 の利用は、
((<"GNU GPL"|URL:http://www.gnu.org/licenses/gpl.html>)) に
従うべきものと考えられます。

=end
