# キーボード拡張アクティビティパッケージ

## 概要

このリポジトリのソースコードは、UiPath Robot　で使用可能なカスタムアクティビティパッケージ

キーボード拡張アクティビティ (UiPathTeam.KeyboardExtension.Activities)

のソースコードです。

キーボード拡張アクティビティの提供する機能は次の通りです。

* 入力メソッドエディター (IME) をオフにする　～　指定したアプリケーションのウィンドウを監視して、IME がオンになるのを阻止 （強制的にオフに） します。文字を入力 (Type Into)　アクティビティで文字を入力する際に、IME　がオンになっていると、入力された文字が IME に入力されるため、意図した入力ができない問題が生じます。この機能により強制的に IME をオフにすることで、その問題を回避することができます。サポートする IME は、日本語、韓国語、中国語です。 

* 強制的に指定のキーボードレイアウトに変更する　～　指定したアプリケーションのウィンドウのキーボードレイアウトを言語 ID で指定したキーボードレイアウトに変更します。ただし、変更できるキーボードレイアウトは、予めインストールされているキーボードレイアウトに限られます。

* 人間により操作されたキーボードまたはマウスまたはその両方の入力をブロックする　～　UiPath Robot による操作のみを許し、ワークフローの実行を不意の入力操作から守ります。

* トグルスイッチとしてキーの組み合わせを監視する　～　上記の機能を一時的に無効したり、有効に戻すトグルスイッチを指定のキー入力の組み合わせで実現します。

## ビルド方法

### 開発ツール

このソースコードは、マイクロソフト Visual Studio Professional 2017 (バージョン 15.9.19) を使って作成しました。ビルドも同じ Visual Studio で行うことを想定します。

### 準備

Visual Studio に加えて、NuGet パッケージファイルをビルドするために nuget.exe コマンドが必要です。

nuget.exe コマンドは、

https://www.nuget.org/downloads

にて入手可能です。nuget.exe　コマンドをこのサイトからダウンロードして tools ディレクトリーに置いてください。

なお、nuget.exe バージョン 5.0 以上を使う場合、マイクロソフト.NET Framework 4.7.2 以上が必要です。詳しくは、

https://docs.microsoft.com/ja-jp/nuget/install-nuget-client-tools

を参照してください。

### 手順

KeyboardExtension.sln ソリューションを Visual Studio で開いて、ソリューションのリビルドによって行います。

ビルドが成功すると、Output というディレクトリが作成され、その中に

UiPathTeam.KeyboardExtension.Activities.x.y.z.nupkg

という名前で NuGet パッケージファイルが作成されます。なお、x.y.z はパッケージのバージョンです。リビルドを重ねると、パッチレベル z が１ずつ増加します。

UiPath Studio のパッケージを管理ウィンドウの設定タブからユーザー定義のパッケージソースとしてこの Output ディレクトリを追加すると、UiPath プロジェクトへのパッケージの取り込みが簡単になります。

## アクティビティの使用方法

### 初期化

まず、キーボード拡張機能を初期化 (Initialize　Keyboard Extension) アクティビティを呼び出し、初期化を行ってください。

### 設定

初期化した後、入力設定を行う (Configure) アクティビティを呼び出して、使用したい機能に関する設定を行ってください。このアクティビティのプロパティは次のとおりです。

* IME を無効化 ～ チェックボックスにチェックを入れると、指定のウィンドウを監視して、IME がオンになると、強制的にオフに戻します。

* トグルシーケンス ～ 入力設定の適用と非適用を交互に切り替えるキーシーケンスを文字列で指定します。文字列には、仮想キー名を受け付けます。プラス記号でキー名を複数指定できます。既定値は左コントロールキーと左シフトキーと右シフトキーを同時に押した場合に切り替わる設定です。

* ブロック (キーボード入力) ～ チェックボックスにチェックを入れると、人間によるキーボード入力をブロックします。

* ブロック (マウス入力) ～ チェックボックスにチェックを入れると、人間によるマウス入力をブロックします。

* 指定キーボードレイアウトに強制変更 ～ チェックボックスにチェックを入れると、指定のウィンドウが入力フォーカスを得たタイミングに、適用したいキーボードレイアウト プロパティで指定されたキーボードレイアウトに強制的に変更します。ただし、変更するキーボードレイアウトは予めインストールされている必要があります。

* 適用したいキーボードレイアウト ～ 言語 ID (整数値) でキーボードレイアウトを指定します。既定値は 1033 （英語・米国） です。 指定キーボードレイアウトに強制変更 プロパティにチェックが入っている場合に適用されるキーボードレイアウトは、このプロパティに完全一致するものが見つからない場合、プライマリー言語に一致するもので最初に見つかったものを採用します。

なお、このアクティビティによる入力設定は、ログオンセッションで唯一の設定になります。次回の呼び出しは、前回の設定を上書きする形になります。

### 適用

入力設定を適用 (Apply) アクティビティを呼び出して、設定した機能を適用したいウィンドウを指定します。このアクティビティのプロパティは次のとおりです。

* ウィンドウハンドル ～ Win32 ウィンドウハンドル (IntPtr値) を指定してください。このプロパティには、IntPtr値だけでなく、次の値を指定することも可能です。

　　* UiPath.Core.Window 値 ～ アプリケーションを開く　(Open Application) アクティビティのアプリケーションウィンドウ プロパティ等で取得可能な値です。

  * UiPath.Core.Browser 値 ～ ブラウザーを開く (Open Browser) アクティビティの UI ブラウザー プロパティ等で取得可能な値です。
  
  * UiPath.Core.UiElement 値 ～ 要素を探す (Find Element) アクティビティの 検出した要素 プロパティ等で取得可能な値です。

* 結果 ～ Boolean 型変数が指定された場合、ウィンドウハンドル プロパティで指定されたウィンドウに設定処理した結果を返します。設定できた場合、True が返り、設定に失敗した場合、False が返ります。

本アクティビティは、異なるウィンドウに対して必要なだけ呼び出すことが可能です。

### 解除

入力設定を解除 (Restore) アクティビティを呼び出して、入力設定を適用 (Apply) アクティビティで指定したウィンドウの指定を解除できます。

* ウィンドウハンドル ～ 入力設定を適用 (Apply) アクティビティで指定したウィンドウで、指定を解除したいウィンドウを指定してください。このプロパティに値を指定することなく、本アクティビティを呼び出すと、現在指定されているすべてのウィンドウが対象になります。

### 終了化

最後に、キーボード拡張機能を終了化 (Uninitialize　Keyboard Extension) アクティビティを呼び出し、終了化を行ってください。
