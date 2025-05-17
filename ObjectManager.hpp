#include "Component.hpp"


// 全てのGameObjectを管理するクラス
class ObjectManager
{
public:

	// 引数の名前のオブジェクトを作成して返す関数
	// 同じ名前のオブジェクトが既に存在していた場合、名前の後ろに番号が付く
	std::shared_ptr<GameObject> GenerateObject(std::string_view a_name)
	{
		// オブジェクトのインスタンスを作成
		std::shared_ptr<GameObject> spNewObject = std::make_shared<GameObject>();
		// 生成するオブジェクトの名前を求める
		std::string objName = CreateObjName(a_name);

		// オブジェクトに名前をセット
		spNewObject->SetName(objName);
		// オブジェクトを有効にする
		spNewObject->SetActive(true);

		// オブジェクトをリストに追加し、そのイテレータを取得
		m_lObjects.emplace_back(spNewObject);
		auto objItr = std::prev(m_lObjects.end());
		// オブジェクトの名前とイテレータを紐づける
		m_umNameToObjPtr[objName] = objItr;

		return spNewObject;
	}

	// 名前からオブジェクトを取得する
	std::weak_ptr<GameObject> GetObject(std::string_view a_name)
	{
		auto itr = m_umNameToObjPtr.find(a_name.data());
		if (itr == m_umNameToObjPtr.end() || *itr->second == nullptr)
		{
			return std::weak_ptr<GameObject>();
		}
		return *itr->second;
	}

	// 更新関数
	void Update()
	{
		// 無効なオブジェクトを全て削除
		RemoveUnActuveObjects();
	}




	// 引数の名前を元に被らない名前を作成する
	std::string CreateObjName(std::string_view a_baseName)
	{
		std::string resultName;

		// 引数の名前のオブジェクトが既に存在しているか調べる
		auto itr = m_umNameToObjPtr.find(a_baseName.data());
		bool isFirstName = false;

		// 初めての名前ならその名前のまま登録
		if (itr == m_umNameToObjPtr.end())
		{
			isFirstName = true;
		}

		// 既に存在する名前なら、名前が重複しないように番号を付ける
		for (size_t i = 1; true; ++i)
		{
			// 有効な名前を見つけたら終了
			if (isFirstName)
			{
				break;
			}

			// 新しい名前を作成
			resultName = a_baseName.data() + std::to_string(i);

			itr = m_umNameToObjPtr.find(resultName);
			// 新しい名前のオブジェクトが存在しなければ
			if (itr == m_umNameToObjPtr.end())
			{
				isFirstName = true;
			}
		}

		return resultName;
	}










	// 無効なオブジェクトを全て削除する
	void RemoveUnActuveObjects()
	{
		for (auto itr = m_lObjects.begin(); itr != m_lObjects.end();)
		{
			// 無効なオブジェクトを削除する
			if (itr->get() == nullptr || !itr->get()->IsActive())
			{
				// オブジェクトのポインタが生きていたら
				if (itr->get() != nullptr)
				{
					// 名前とイテレータの情報を削除
					m_umNameToObjPtr.erase(itr->get()->GetName().data());
				}

				// オブジェクトのインスタンスを削除
				itr = m_lObjects.erase(itr);
			}
			// 有効なオブジェクトなら何もせず次のオブジェクトを調べる
			else
			{
				itr++;
			}
		}
	}



	// 全てのオブジェクトを強制的に解放 (ゲーム終了時など)
	void ReleaseAllObjects() {
		std::cout << "[ObjectManager] Releasing all objects..." << std::endl;
		for(const auto& obj : m_lObjects) {
			if(obj) {
				// GameObject 内部のコンポーネントの OnRelease を呼ぶような仕組みがあっても良い
				// ここでは ObjectManager が直接 GameObject を解放する
				std::cout << "[ObjectManager] Releasing components for: " << obj->GetName() << std::endl;
				// GameObject のデストラクタでコンポーネントの shared_ptr が解放されることを期待
			}
		}
		m_lObjects.clear();
		m_umNameToObjPtr.clear();
		std::cout << "[ObjectManager] All objects released." << std::endl;
	}
private:
	void RemoveInactiveObjects() {
		// m_umNameToObjPtr から先に削除
		for(auto it = m_lObjects.begin(); it != m_lObjects.end(); /* no increment */) {
			if(*it && !(*it)->IsActive()) {
				std::cout << "[ObjectManager] Removing inactive object: " << (*it)->GetName() << std::endl;
				m_umNameToObjPtr.erase((*it)->GetName());
				// OnRelease を呼びたい場合はここで呼ぶか、GameObject のデストラクタでコンポーネントが解放される際に呼ばれるようにする
				// (*it)->CallAllComponentsOnRelease(); // 例えばこんなメソッドをGameObjectに用意する
				it = m_lObjects.erase(it); // erase は次の有効なイテレータを返す
			}
			else {
				++it;
			}
		}
	}

	// オブジェクトの名前とイテレータを紐づけるコンテナ
	std::unordered_map<std::string, std::list<std::shared_ptr<GameObject>>::iterator> m_umNameToObjPtr;

	// 全てのオブジェクトのインスタンスを格納するコンテナ
	std::list<std::shared_ptr<GameObject>> m_lObjects;

};