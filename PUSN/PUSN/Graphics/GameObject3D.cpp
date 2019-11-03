#include "GameObject3D.h"

void GameObject3D::SetLookAtPos(XMFLOAT3 lookAtPos)
{
	//Verify that look at pos is not the same as cam pos. They cannot be the same as that wouldn't make sense and would result in undefined behavior.
	if (lookAtPos.x == pos.x && lookAtPos.y == pos.y && lookAtPos.z == pos.z)
		return;

	lookAtPos.x = pos.x - lookAtPos.x;
	lookAtPos.y = pos.y - lookAtPos.y;
	lookAtPos.z = pos.z - lookAtPos.z;

	float pitch = 0.0f;
	if (lookAtPos.y != 0.0f)
	{
		const float distance = sqrt(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);
		pitch = atan(lookAtPos.y / distance);
	}

	float yaw = 0.0f;
	if (lookAtPos.x != 0.0f)
	{
		yaw = atan(lookAtPos.x / lookAtPos.z);
	}
	if (lookAtPos.z > 0)
		yaw += XM_PI;

	this->SetRotation(pitch, yaw, 0.0f);
}

const XMVECTOR& GameObject3D::GetForwardVector(bool omitZ)
{
	if (omitZ)
		return vec_forward_noZ;
	else
		return vec_forward;
}

const XMVECTOR& GameObject3D::GetRightVector(bool omitZ)
{
	if (omitZ)
		return vec_right_noZ;
	else
		return vec_right;
}

const XMVECTOR& GameObject3D::GetBackwardVector(bool omitZ)
{
	if (omitZ)
		return vec_backward_noZ;
	else
		return vec_backward;
}

const XMVECTOR& GameObject3D::GetLeftVector(bool omitZ)
{
	if (omitZ)
		return vec_left_noZ;
	else
		return vec_left;
}

void GameObject3D::UpdateMatrix()
{
	assert("UpdateMatrix must be overridden." && 0);
}

void GameObject3D::UpdateDirectionVectors()
{
	XMMATRIX vecRotationMatrix = XMMatrixRotationX(rot.x) * XMMatrixRotationZ(rot.z);
	vec_forward = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vecRotationMatrix);
	vec_backward = XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vecRotationMatrix);
	vec_left = XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vecRotationMatrix);
	vec_right = XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vecRotationMatrix);

	XMMATRIX vecRotationMatrixnoZ = XMMatrixRotationZ(rot.z);
	vec_forward_noZ = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vecRotationMatrixnoZ);
	vec_backward_noZ = XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vecRotationMatrixnoZ);
	vec_left_noZ = XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vecRotationMatrixnoZ);
	vec_right_noZ = XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vecRotationMatrixnoZ);
}