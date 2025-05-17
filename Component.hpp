#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector> // typeid のために必要になる場合がある (コンパイラによる)
#include <typeinfo> // typeid のために必要



// 前方宣言
class ObjectManager; // GameObject が ObjectManager をフレンドクラスとして宣言するため
class GameObject;


class ComponentBase
{
public:
    virtual ~ComponentBase() = default; // 仮想デストラクタを追加

    // コンポーネントの開始時処理の仮想関数
    // 初めて更新される直前に呼ばれる
    virtual void OnStart() {}

    // 通常の更新の前に更新される処理の仮想関数
    virtual void OnPreUpdate() {}

    // 通常の更新する処理の仮想関数
    virtual void OnUpdate() {}

    // 通常の更新の後に更新する処理の仮想関数
    virtual void OnPostUpdate() {}

    // コンポーネントが解放されるときの処理の仮想関数
    virtual void OnRelease() {}


    // このコンポーネントの持ち主を取得
    std::weak_ptr<GameObject> GetOwner() const // const修飾子を追加
    {
        return m_wpOwner; // m_spOwner から m_wpOwner に変更
    }

private:
    friend class GameObject; // GameObjectからSetOwnerを呼べるようにする

    // このコンポーネントの持ち主をセット
    void SetOwner(std::shared_ptr<GameObject> a_spOwner)
    {
        m_wpOwner = a_spOwner; // shared_ptr から weak_ptr へ代入
    }

private:
    // このコンポーネントの持ち主 (GameObjectとの循環参照を避けるためweak_ptrにする)
    std::weak_ptr<GameObject> m_wpOwner;
};




class GameObject: public std::enable_shared_from_this<GameObject> // SetOwnerでthisをshared_ptrとして渡すため
{
public:
    //---------------------------------
    // Component
    //---------------------------------

    // 引数のコンポーネントをアタッチする関数
    // 引数の名前はRTTIが許される環境ならtypeidなどを使うとよい
   void AddComponent(std::shared_ptr<ComponentBase> a_spComponent,std::string_view a_name)
    {
        // コンポーネントの持ち主としてこのオブジェクトをセット
        // a_spComponent->SetOwner(this); // 直接thisを渡すのは危険。shared_from_this()を使う
        a_spComponent->SetOwner(shared_from_this());


        // コンポーネントのインスタンスを名前と紐づけて保存
        m_umNameToComp[std::string(a_name)] = a_spComponent; // string_viewからstringへ変換
    }


    // 引数の名前のコンポーネントを解放し削除する関数
    void RemoveComponent(std::string_view a_compName)
    {
        auto itr = m_umNameToComp.find(std::string(a_compName)); // string_viewからstringへ変換

        // 引数の名前のコンポーネントが無効なら終了
        if(itr == m_umNameToComp.end())
        {
            return;
        }
        if(itr->second == nullptr)
        {
            return;
        }

        // コンポーネントの解放処理を呼ぶ
        itr->second->OnRelease();

        // コンポーネントのインスタンスを削除
        m_umNameToComp.erase(std::string(a_compName)); // string_viewからstringへ変換
    }

    // コンポーネントを名前から取得
    std::weak_ptr<ComponentBase> GetComponent(std::string_view a_name)
    {
        auto itr = m_umNameToComp.find(std::string(a_name)); // string_viewからstringへ変換
        if(itr == m_umNameToComp.end())
        {
            return std::weak_ptr<ComponentBase>();
        }

        return itr->second;
    }

    // コンポーネントを追加する (テンプレート版)
    // 引数からコンストラクタに値を代入することができる
    template<typename CompType,typename...ArgTypes>
    std::weak_ptr<CompType> AddComponent(ArgTypes&&... a_args) // 戻り値を CompType の weak_ptr に変更、引数を完全転送
    {
        // CompType が ComponentBase から派生しているかチェック (任意)
        static_assert(std::is_base_of<ComponentBase,CompType>::value,"CompType must derive from ComponentBase");

        // コンポーネントのインスタンスを作成
        std::shared_ptr<CompType> spNewComp = std::make_shared<CompType>(std::forward<ArgTypes>(a_args)...);

        // ComponentBaseへのポインタも取得しておく
        std::shared_ptr<ComponentBase> spNewCompBase = spNewComp;

        // コンポーネントの持ち主としてこのオブジェクトをセット
        spNewCompBase->SetOwner(shared_from_this());

        // コンポーネントのインスタンスを名前と紐づけて保存
        // typeid(CompType).name() は環境によって異なる mangled name を返す可能性があるため注意
        m_umNameToComp[typeid(CompType).name()] = spNewCompBase;

        return spNewComp; // CompType の weak_ptr を返す
    }

    // 引数の型のコンポーネントを解放し削除する関数 (テンプレート版)
    template<typename CompType>
    void RemoveComponent()
    {
        // CompType が ComponentBase から派生しているかチェック (任意)
        static_assert(std::is_base_of<ComponentBase,CompType>::value,"CompType must derive from ComponentBase");

        std::string compName = typeid(CompType).name();
        auto itr = m_umNameToComp.find(compName);

        // 引数の名前のコンポーネントが無効なら終了
        if(itr == m_umNameToComp.end())
        {
            return;
        }
        if(itr->second == nullptr)
        {
            return;
        }

        // コンポーネントの解放処理を呼ぶ
        itr->second->OnRelease();

        // コンポーネントのインスタンスを削除
        m_umNameToComp.erase(compName);
    }

    // コンポーネントを型から取得 (テンプレート版)
    template<typename CompType>
    std::weak_ptr<CompType> GetComponent() // 戻り値を CompType の weak_ptr に変更
    {
        // CompType が ComponentBase から派生しているかチェック (任意)
        static_assert(std::is_base_of<ComponentBase,CompType>::value,"CompType must derive from ComponentBase");

        auto itr = m_umNameToComp.find(typeid(CompType).name());
        if(itr == m_umNameToComp.end() || itr->second == nullptr)
        {
            return std::weak_ptr<CompType>();
        }

        // ComponentBase の shared_ptr から CompType の shared_ptr へ動的キャスト
        // キャストに失敗した場合は nullptr を持つ weak_ptr が返る
        return std::dynamic_pointer_cast<CompType>(itr->second);
    }


    //---------------------------------
    // Status
    //---------------------------------

    // オブジェクトの有効状態をセットする
    void SetActive(bool a_isActive)
    {
        m_isActive = a_isActive;
    }

    bool IsActive() const // CheckActive から IsActive に変更し、const修飾子を追加
    {
        return m_isActive;
    }

    const std::string& GetName() const // 戻り値を const std::string& に変更し、const修飾子を追加
    {
        return m_name;
    }

private:
    friend class ObjectManager; // ObjectManagerから private メンバにアクセス許可

    // 名前をセットする (ObjectManagerからのみ呼ばれることを想定)
    void SetName(std::string_view a_name)
    {
        m_name = a_name;
    }


    //---------------------------------
    // 更新 (ObjectManagerから呼ばれることを想定)
    //---------------------------------
public: // ObjectManagerから呼び出せるようにpublicに変更 (またはObjectManagerをfriendにする)
    // もしGameObject自身が更新ループを持つならprivateのままでも良い

// 通常の更新の前に呼ぶ処理
    void PreUpdate()
    {
        if(!m_isActive) return; // 非アクティブなら何もしない

        // 初めての更新ならStartを呼ぶ
        if(!m_isCalledUpdate)
        {
            // OnStart中にコンポーネントが追加/削除される可能性を考慮し、イテレータが無効にならないように注意
            // 一度キーを収集してから処理するなどの対策が考えられるが、ここではシンプルに直接ループ
            for(auto& pair : m_umNameToComp) // 範囲for文の参照を修正
            {
                if(pair.second) pair.second->OnStart();
            }
            m_isCalledUpdate = true;
        }

        // 全てのコンポーネントのPreUpdateを呼ぶ
        for(auto& pair : m_umNameToComp) // 範囲for文の参照を修正
        {
            if(pair.second) // nullptrチェック
            {
                pair.second->OnPreUpdate();
            }
        }
    }

    // 通常の更新処理
    void Update()
    {
        if(!m_isActive) return; // 非アクティブなら何もしない

        // 全てのコンポーネントのUpdateを呼ぶ
        for(auto& pair : m_umNameToComp) // 範囲for文の参照を修正
        {
            if(pair.second) // nullptrチェック
            {
                pair.second->OnUpdate();
            }
        }
    }

    // 通常の更新の後に呼ぶ
// 通常の更新の後に呼ぶ処理
    void PostUpdate()
    {
        if(!m_isActive) return; // 非アクティブなら何もしない

        // 全てのコンポーネントのPostUpdateを呼ぶ
        for(auto& pair : m_umNameToComp) // 範囲for文の参照を修正
        {
            if(pair.second) // nullptrチェック
            {
                pair.second->OnPostUpdate();
            }
        }
    }

    // GameObjectが破棄される際に、保持している全コンポーネントのOnReleaseを呼ぶ
    // ObjectManagerなどでGameObjectがリストから削除される直前などに明示的に呼ぶか、
    // GameObjectのデストラクタで呼ぶことを検討。
    // ここではデストラクタで呼ぶ例を示す。
 
   ~GameObject() {
         std::cout << "[GameObject] Destructor for: " << m_name << std::endl;
        for(auto& pair : m_umNameToComp) {
            if(pair.second) {
                // std::cout << "[GameObject] Calling OnRelease for component in " << m_name << std::endl;
                pair.second->OnRelease();
            }
        }
        m_umNameToComp.clear(); // shared_ptrが解放される
    }


private:

    // 既に更新が呼ばれているか
    bool m_isCalledUpdate = false;

    // オブジェクトが有効か
    bool m_isActive = false; // デフォルトはfalseが良いかもしれない（生成後SetActive(true)で有効化）

    // オブジェクトの名前
    std::string m_name;

    // コンポーネントの名前(typeid().name() または指定した名前)とインスタンスを紐づけて格納するコンテナ
    // キーを std::string に統一
    std::unordered_map<std::string,std::shared_ptr<ComponentBase>> m_umNameToComp;

};
/*
template<typename CompType,typename...ArgTypes>
std::weak_ptr<ComponentBase> AddComponent(ArgTypes... a_args)
{
    // コンポーネントのインスタンスを作成
    std::shared_ptr<ComponentBase> spNewComp = std::make_shared<CompType>(a_args...);

    // コンポーネントの持ち主としてこのオブジェクトをセット
    spNewComp->SetOwner(this);

    // コンポーネントのインスタンスを名前と紐づけて保存
    m_umNameToComp[typeid(CompType).name()] = spNewComp;
}
*/
#endif // GAMEOBJECT_HPP