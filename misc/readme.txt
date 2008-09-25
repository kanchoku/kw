このディレクトリには雑多なファイルを置いています。

■ 部首合成変換関連

    hanzen.rev

	部首合成変換辞書のための、半角/全角文字の等価定義。
	tc2 に付属の bushu.rev, symbol.rev とともに、このファイルを
	マージして作成したのが、本パッケージに付属する kwbushu.rev です。
	なお、1 行目は半角空白/全角空白の等価定義のエントリです。

■ 交ぜ書き変換関連

    t-tut-2st.uncertain

	T-Code または TUT-Code で、2 打鍵で入力できる漢字 (または漢字に
	準じる文字) 計 1183 文字のリスト。
	このファイルを uncertain に、空ファイルを certain に指定して
	tc2 の交ぜ書き変換辞書作成キットを用いて作成した交ぜ書き変換辞書が、
	本パッケージに付属する kwmaze.dic です。
	なお tc-2.3.1 での交ぜ書き変換辞書作成の手順は次のとおり:
	(1) tcode-data-directory に certain と uncertain というファイルを
	    用意する
	(2) Emacs にて `M-x tcode-make-mazegaki-dictionary' を実行
	(3) ~/mazegaki.dic が生成される

■ 熟語ガイド機能関連

    gg1.rb
    gg2.rb

	『【漢直】T-Code/TUT-Codeスレ Lesson2【連習】』
	<http://pc.2ch.net/test/read.cgi/unix/1061936143/>
	の 208-212 さんによる、
	熟語ガイド辞書 kwgg.dic (新形式) を作成する Ruby スクリプトです。

    reducegg.rb

	kwgg.dic から (certain ファイルを指定して) 覚えた漢字を削除する
	Ruby スクリプトです。

    mkgg.rb

	旧形式の熟語ガイド辞書を作成する Ruby スクリプトです (非推奨)。

■ 漢索窓関連

    kana.certain

	JIS X0208 のひらがな 83 文字とかたかな 86 文字のリスト。
	「漢索窓」の使用の際、設定ファイル kanchoku.ini の [kansaku] 
	セクションにて、「certain=kana.certain」のように指定して
	使用することを想定しています。

