#include <shared/spacemouse_spnav.h>
#include <shared/stereo_camera.h>
#include <spnav.h>

using namespace all;

SpacemouseSpnav::SpacemouseSpnav(all::StereoCamera* camera, std::shared_ptr<all::ModelNavParameters> p)
    : Spacemouse(camera, p)
{
    if (spnav_open() == -1) {
        // qCDebug(spcms) << "could not connect to spacenavd";
    } else {
        m_poller = std::thread{ [this]() {
            while (!stop) {
                // qCDebug(spcms) << "polling";
                poll();
            }
            // qCDebug(spcms) << "STOP!";
        } };
        m_poller.detach();
    }
}

SpacemouseSpnav::~SpacemouseSpnav()
{
    stop = true;
    if (m_poller.joinable())
        m_poller.join();
    spnav_close();
}

void SpacemouseSpnav::poll()
{
    // qCDebug(spcms) << "poll";
    spnav_event sev;
    if (spnav_wait_event(&sev)) {
        const glm::mat4x4 viewCenter = m_camera->cameraMatrix();

        const glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), sev.motion.rx * m_rotFactor, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), sev.motion.ry * m_rotFactor, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), sev.motion.rz * m_rotFactor, glm::vec3(0.0f, 0.0f, 1.0f));

        // Combine the rotation matrices
        const glm::mat4 combinedRotation = rotateX * rotateY * rotateZ;

        // Apply the combined rotation to the original view matrix
        glm::mat4 rotatedViewMatrix = combinedRotation * viewCenter;

        // qCDebug(spcms) << "spcms event" << viewCenter;
        switch (sev.type) {
        case SPNAV_EVENT_MOTION:
            // qCDebug(spcms) << sev.motion.rx << sev.motion.ry << sev.motion.rz;
            rotatedViewMatrix[3] = { m_camera->position() + glm::vec3{ sev.motion.x, sev.motion.y, sev.motion.z } * m_translFactor, 1 };
            m_camera->setCameraMatrix(rotatedViewMatrix);

            break;
        case SPNAV_EVENT_BUTTON:
            /* 0-based button number in sev.button.bnum.
             * button state in sev.button.press (non-zero means pressed).
             */
            break;
        default:;
        }
    }
}
