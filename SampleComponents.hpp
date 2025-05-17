#ifndef SAMPLE_COMPONENTS_HPP
#define SAMPLE_COMPONENTS_HPP

#include "Component.hpp" // ComponentBase, GameObject を使うため
#include <iostream>
#include <cmath> // For std::sin, std::cos

// 位置情報を持ち、移動するコンポーネント
class TransformComponent : public ComponentBase {
public:
    float x = 0.0f;
    float y = 0.0f;
    float speed = 50.0f; // 移動速度（角度変化の速さなど）
    float radius = 5.0f; // 円運動の半径
    float current_angle_deg = 0.0f; // 現在の角度（度数法）

    TransformComponent(float startX = 0.0f, float startY = 0.0f, float s = 50.0f, float r = 5.0f)
        : x(startX), y(startY), speed(s), radius(r), initialX(startX), initialY(startY) {}

    void OnStart() override {
        if (auto owner = GetOwner().lock()) {
            std::cout << "[" << owner->GetName() << ".Transform] Started. Initial Pos: (" << x << ", " << y << "), Speed: " << speed << ", Radius: " << radius << std::endl;
        }
    }

    void OnUpdate() override {
        // 毎フレーム角度を更新
        current_angle_deg += speed * 0.016f; // 0.016f は約60FPS時の1フレーム時間と仮定
        if (current_angle_deg >= 360.0f) {
            current_angle_deg -= 360.0f;
        }

        // 新しい位置を計算 (初期位置を中心とした円運動)
        float angle_rad = current_angle_deg * (3.1415926535f / 180.0f);
        x = initialX + radius * std::cos(angle_rad);
        y = initialY + radius * std::sin(angle_rad);
    }

    void OnRelease() override {
        if (auto owner = GetOwner().lock()) {
            std::cout << "[" << owner->GetName() << ".Transform] Released." << std::endl;
        }
    }
private:
    float initialX; // 円運動の中心とするための初期位置
    float initialY;
};

// オブジェクト情報を表示するコンポーネント
class RendererComponent : public ComponentBase {
public:
    void OnPostUpdate() override { // Update後の方が位置が確定している
        auto owner_sp = GetOwner().lock(); // weak_ptr から shared_ptr を取得
        if (!owner_sp) return;

        // TransformComponent を取得試行
 
       auto transform_wp = owner_sp->GetComponent<TransformComponent>();
        auto transform_sp = transform_wp.lock();

        if (transform_sp) {
            std::cout << "[" << owner_sp->GetName() << ".Renderer] Displaying at Pos: ("
                      << transform_sp->x << ", " << transform_sp->y << ")" << std::endl;
        } else {
            std::cout << "[" << owner_sp->GetName() << ".Renderer] (No TransformComponent to display position)" << std::endl;
        }

    }

    void OnRelease() override {
        if (auto owner = GetOwner().lock()) {
            std::cout << "[" << owner->GetName() << ".Renderer] Released." << std::endl;
        }
    }
};

// 特定の条件でGameObjectを非アクティブにするコンポーネント (入力シミュレーション)
class PlayerInputSimulatorComponent : public ComponentBase {
public:
    int frame_to_deactivate = 0;
    int current_frame = 0;

    PlayerInputSimulatorComponent(int deactivate_at_frame) : frame_to_deactivate(deactivate_at_frame) {}

    void OnStart() override {
        if (auto owner = GetOwner().lock()) {
            std::cout << "[" << owner->GetName() << ".InputSim] Started. Will deactivate owner at frame " << frame_to_deactivate << std::endl;
        }
    }

    void OnUpdate() override {
        current_frame++;
        auto owner_sp = GetOwner().lock();
        if (!owner_sp) return;

        if (current_frame >= frame_to_deactivate) {
            if (owner_sp->IsActive()) { // まだアクティブなら
                std::cout << "[" << owner_sp->GetName() << ".InputSim] Frame " << current_frame
                          << " reached. Deactivating owner." << std::endl;
                owner_sp->SetActive(false); // GameObject を非アクティブにする
            }
        }
    }

    void OnRelease() override {
        if (auto owner = GetOwner().lock()) {
            std::cout << "[" << owner->GetName() << ".InputSim] Released." << std::endl;
        }
    }
};

#endif // SAMPLE_COMPONENTS_HPP