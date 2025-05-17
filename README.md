c++17

元ネタhttps://zenn.dev/kd_gamegikenblg/articles/8fbf1df73f52ce
「C++でコンポーネント指向設計のオブジェクト管理を実装する方法」

変更点
typeidを使いコンポーネット型名と名前で検索するように変更した。この方が速くなるらしいとAIが。
他もろもろ。元々↑のだけではコンパイル通らないのをちゃんと動く様にした

注意点
enemy->GetComponent<TransformComponent>().lock();
GetComponentする時はshare_ptrからweak_ptrに弱参照するので.lock()が必要

