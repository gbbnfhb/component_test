#include <iostream>
#include <string>
#include <memory>
#include "Component.hpp" // GameObject.hpp をインクルード
#include "ObjectManager.hpp" // ObjectManager.hpp をインクルード
#include "SampleComponents.hpp" // ObjectManager.hpp をインクルード



int main() {
    ObjectManager objectManager;

    // --- Playerオブジェクトの作成 ---
    std::shared_ptr<GameObject> player = objectManager.GenerateObject("Player");

    if(player) {
        // TransformComponent を追加 (コンストラクタ引数なし)
        player->AddComponent<TransformComponent>();

        // RendererComponent を追加
        player->AddComponent<RendererComponent>();

        // 初期位置を設定したい場合 (AddComponent後に取得して設定)
        auto transformComp = player->GetComponent<TransformComponent>().lock();
        if(transformComp) {
            static_cast<TransformComponent*>(transformComp.get())->x = 10.0f;
            static_cast<TransformComponent*>(transformComp.get())->y = 5.0f;
        }
    }

    // --- Enemyオブジェクトの作成 ---
    std::shared_ptr<GameObject> enemy = objectManager.GenerateObject("Enemy");
    if(enemy) {
        // TransformComponent を追加し、初期位置をコンストラクタで設定
        // (注意: この機能は提供されたコードのテンプレート版 AddComponent を使う必要があります)
         enemy->AddComponent<TransformComponent>(50.0f, 100.0f); // この行はコメントアウトされたテンプレート版AddComponent向け
                                                               // 現在のAddComponentでは引数を渡せません。

        // 現在のAddComponentを使う場合：
/*
        enemy->AddComponent<TransformComponent>();
        auto enemyTransform = enemy->GetComponent<TransformComponent>().lock();
        if(enemyTransform) {
            static_cast<TransformComponent*>(enemyTransform.get())->x = 50.0f;
            static_cast<TransformComponent*>(enemyTransform.get())->y = 100.0f;
        }
*/
        enemy->AddComponent<RendererComponent>();
    }

    // --- ゲームループのシミュレーション ---
    std::cout << "\n--- Simulating Game Loop (5 frames) ---" << std::endl;
    for(int i = 0; i < 5; ++i) {
        std::cout << "\n--- Frame " << i + 1 << " ---" << std::endl;

        // ObjectManager の更新 (現在は主に無効オブジェクトの削除)
        objectManager.Update();

        // 全ての有効なGameObjectの更新処理を呼び出す (ObjectManagerにこの機能を追加すると良い)
        // ここでは手動で呼び出します
        if(player && player->IsActive()) {
            player->PreUpdate();
            player->Update();
            player->PostUpdate();
        }
        if(enemy && enemy->IsActive()) {
            enemy->PreUpdate();
            enemy->Update();
            enemy->PostUpdate();
        }

        // 2フレーム目にEnemyを非アクティブにしてみる
        if(i == 1 && enemy) {
            std::cout << "\n--- Deactivating Enemy ---" << std::endl;
            enemy->SetActive(false);
        }
    }

    // --- 特定のコンポーネントを削除する例 ---
    if(player) {
        std::cout << "\n--- Removing RendererComponent from Player ---" << std::endl;
        player->RemoveComponent<RendererComponent>(); // テンプレート版を使用
        // player->RemoveComponent("RendererComponent"); // 文字列版を使用する場合 (RTTIに依存しない)
    }

    // --- ゲームループのシミュレーション (コンポーネント削除後) ---
    std::cout << "\n--- Simulating Game Loop After Component Removal (2 frames) ---" << std::endl;
    for(int i = 0; i < 2; ++i) {
        std::cout << "\n--- Frame " << i + 6 << " ---" << std::endl;
        objectManager.Update();
        if(player && player->IsActive()) {
            player->PreUpdate();
            player->Update();
            player->PostUpdate();
        }
        // Enemyは非アクティブなので更新されない
    }


    // --- オブジェクトの取得 ---
    std::cout << "\n--- Getting Objects ---" << std::endl;
    std::weak_ptr<GameObject> foundPlayer = objectManager.GetObject("Player");
    if(auto p = foundPlayer.lock()) {
        std::cout << "Found object: " << p->GetName() << std::endl;
    }
    else {
        std::cout << "Player object not found (or already released)." << std::endl;
    }

    std::weak_ptr<GameObject> foundEnemy = objectManager.GetObject("Enemy");
    if(auto e = foundEnemy.lock()) {
        std::cout << "Found object: " << e->GetName() << std::endl;
    }
    else {
        // EnemyはSetActive(false)にした後、ObjectManager::Update()で削除されているはず
        std::cout << "Enemy object not found (likely removed as inactive)." << std::endl;
    }


    // --- 全オブジェクトの解放 ---
    std::cout << "\n--- Releasing All Objects ---" << std::endl;
    objectManager.ReleaseAllObjects();

    std::cout << "\n--- Program End ---" << std::endl;

    return 0;
}