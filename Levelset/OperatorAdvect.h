#pragma once

#include "Levelset/LevelSetOperator.h"
#include "Math/Function3D.h"

/*! \brief A level set operator that does external advection
 *
 * This class implements level set advectionr in an external vector field by the
 * PDE
 *
 *  \f$
 *  \dfrac{\partial \phi}{\partial t} + \mathbf{V}(\mathbf{x})\cdot \nabla \phi
 * = 0 \f$
 */
//! \lab4 Implement advection in external vector field
class OperatorAdvect : public LevelSetOperator {
protected:
    Function3D<glm::vec3> *mVectorField;

public:
    OperatorAdvect(LevelSet *LS, Function3D<glm::vec3> *vf)
        : LevelSetOperator(LS), mVectorField(vf) {}

    virtual float ComputeTimestep() {
        // Compute and return a stable timestep
        // (Hint: Function3D::GetMaxValue())
        float dx = mLS->GetDx();
        //float V = glm::length(mVectorField->GetMaxValue());
        glm::vec3 M(glm::abs(mVectorField->GetMaxValue()));
        float V = glm::max(M.x, M.y);
        V = glm::max(V, M.z);
        return (dx/V) * 0.9;
    }

    virtual void Propagate(float time) {
        // Determine timestep for stability
        float dt = ComputeTimestep();

        // Propagate level set with stable timestep dt
        // until requested time is reached
        for (float elapsed = 0; elapsed < time;) {
            if (dt > time - elapsed) dt = time - elapsed;
            elapsed += dt;

            IntegrateEuler(dt);
            // IntegrateRungeKutta(dt);
        }
    }

    virtual float Evaluate(size_t i, size_t j, size_t k) {
        // Compute the rate of change (dphi/dt)

        // Remember that the point (i,j,k) is given in grid coordinates, while
        // the velocity field used for advection needs to be sampled in
        // world coordinates (x,y,z). You can use LevelSet::TransformGridToWorld()
        // for this task.
        float x = (float)i;
        float y = (float)j;
        float z = (float)k;

        mLS->TransformGridToWorld(x, y, z);

        // sampled in world coordinates
        glm::vec3 v = mVectorField->GetValue(x, y, z);

        glm::vec3 gradient;
        gradient.x = v.x < 0 ? mLS->DiffXp(i, j, k) : mLS->DiffXm(i, j, k);
        gradient.y = v.y < 0 ? mLS->DiffYp(i, j, k) : mLS->DiffYm(i, j, k);
        gradient.z = v.z < 0 ? mLS->DiffZp(i, j, k) : mLS->DiffZm(i, j, k);

        return glm::dot(-v, gradient);
    }
};