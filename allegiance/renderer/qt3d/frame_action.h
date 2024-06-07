#pragma once
#include <Qt3DLogic/QFrameAction>

namespace all::qt3d {
class FrameAction : public Qt3DLogic::QFrameAction
{
    Q_OBJECT
public:
    explicit FrameAction(Qt3DCore::QNode* parent = nullptr)
        : Qt3DLogic::QFrameAction(parent)
    {
        connect(this, &Qt3DLogic::QFrameAction::triggered, this, &FrameAction::countAndTrigger);
    }

public Q_SLOTS:
    void countAndTrigger(float dt)
    {
        if (!callbacks.size())
            return;

        auto it = callbacks.begin();
        while (it != callbacks.end()) {
            it->first--;

            if (it->first < 1) {
                it->second();
                it = callbacks.erase(it); // erase returns the iterator to the next valid position
            } else {
                ++it; // move to the next element
            }
        }
    }

public:
    QVector<QPair<int, std::function<void()>>> callbacks;
};
} // namespace all::qt3d
