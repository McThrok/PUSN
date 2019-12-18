#include "Model.h"


void Model::LoadElephant(float minZ)
{
	model0.surfaces.clear();

	char cCurrentPath[FILENAME_MAX];
	_getcwd(cCurrentPath, sizeof(cCurrentPath));
	string path = std::string(cCurrentPath) + "\\Models\\";
	string filePath = path + "wt_elephant.mg1";

	ifstream file(filePath);
	string line;
	while (getline(file, line)) {

		vector<string> header;
		StringHelper::Split(line, header);

		if (header.empty())
			continue;

		string elementName = header[0];
		int n = stoi(header[1]);

		for (int j = 0; j < n; j++)
		{
			getline(file, line);

			if (elementName == "surfaceC0")
				model0.surfaces.push_back(make_shared<BezierSurfaceC0>(line));
			else if (elementName == "tubeC0")
				model0.surfaces.push_back(make_shared<BezierSurfaceC0>(line, true));
		}
	}

	AdjustEar();
	Rescale(minZ);
	AddRescaledModelVersion();
}

void Model::Rescale(float minZ)
{
	Matrix modelTransform = XMMatrixScaling(4.5f, 4.5f, 9.0f) * XMMatrixTranslation(-5, 0, minZ);

	for (int k = 0; k < model0.surfaces.size(); k++)
	{
		BezierSurfaceC0* surf = model0.surfaces[k].get();
		for (int i = 0; i < surf->GetWidthVertexCount(); i++)
		{
			for (int j = 0; j < surf->GetHeightVertexCount(); j++)
			{
				XMStoreFloat3(&surf->GetVert(i, j), XMVector3TransformCoord(XMLoadFloat3(&surf->GetVert(i, j)), modelTransform));
			}
		}
	}
}
void Model::AdjustEar()
{
	Matrix modelTransform = XMMatrixScaling(1.0f, 1.0f, 0.5f) * XMMatrixTranslation(0, 0, 1);

	BezierSurfaceC0* surfaces[2] = { model0.GetLeftEar(),model0.GetRightEar() };
	for (int k = 0; k < 2; k++)
	{
		BezierSurfaceC0* surf = surfaces[k];
		for (int i = 0; i < surf->GetWidthVertexCount(); i++)
		{
			for (int j = 0; j < surf->GetHeightVertexCount(); j++)
			{
				XMStoreFloat3(&surf->GetVert(i, j), XMVector3TransformCoord(XMLoadFloat3(&surf->GetVert(i, j)), modelTransform));
			}
		}
	}

}
void Model::AddRescaledModelVersion()
{
	model8 = ModelVersion(model0);
	ChangeSizeAlongNormals(model8, 4);
}

void Model::ChangeSizeAlongNormals(ModelVersion& model, float length)
{
	auto surfaces = model.GetSurfaces();

	for (int i = 0; i < surfaces.size(); i++)
	{
		BezierSurfaceC0& bs = *surfaces[i];
		int wc = bs.GetWidthVertexCount();
		int hc = bs.GetHeightVertexCount();
		vector<Vector3> tmp(wc * hc);

		for (int w = 0; w < wc; w++)
			for (int h = 0; h < hc; h++)
			{
				Vector2 hw = bs.GetVertParametrization(w, h);
				Vector3 change = bs.EvaluateNormal(hw);
				float dist = length / 0.85;
				if (w % 3 != 0)
					dist *= 1.3;

				tmp[w * hc + h] = bs.GetVert(w, h) + dist * change;
			}

		for (int w = 0; w < wc; w++)
			for (int h = 0; h < hc; h++)
				bs.GetVert(w, h) = tmp[w * hc + h];

		if (bs.isCylinder && &bs != model.GetTail())
		{
			for (int h = 0; h < hc; h++)
			{
				Vector3 avg = (bs.GetVert(0, h) + bs.GetVert(bs.GetWidthVertexCount() - 1, h)) / 2;
				bs.GetVert(0, h) = avg;
				bs.GetVert(bs.GetWidthVertexCount() - 1, h) = avg;
			}
		}
	}

	for (int w = 0; w < 7; w++)
		for (int h = 2; h < 4; h++)
			model.GetHead()->GetVert(w, h) += {-1.2, -1.2, 0};

	BezierSurfaceC0* surf = model.GetTorso();
	for (int w = 0; w < surf->GetHeightVertexCount(); w++)
	{
		surf->GetVert(w, 0).x += length;
		surf->GetVert(w, surf->GetHeightVertexCount() - 1).x -= length;
	}

	BezierSurfaceC0* legs[2] = { model.GetLeftEar(),model.GetRightEar() };
	for (int k = 0; k < 2; k++)
	{
		BezierSurfaceC0* surf = legs[k];
		int wc = surf->GetWidthVertexCount();
		int hc = surf->GetHeightVertexCount();


		for (int i = 0; i <= wc / 2; i++)
		{
			{
				Vector3& v1 = surf->GetVert(i, 0);
				Vector3& v2 = surf->GetVert(wc - 1 - i, 0);
				Vector3 avg = (v1 + v2) / 2;
				v1 = avg;
				v2 = avg;
			}

			{
				Vector3& v1 = surf->GetVert(i, hc - 1);
				Vector3& v2 = surf->GetVert(wc - 1 - i, hc - 1);
				Vector3 avg = (v1 + v2) / 2;
				v1 = avg;
				v2 = avg;
			}
		}
	}
}

vector<BezierSurfaceC0*> ModelVersion::GetSurfaces()
{
	vector<BezierSurfaceC0*> result(8);

	for (int i = 0; i < surfaces.size(); i++)
		result[i] = surfaces[i].get();

	return result;
}

ModelVersion::ModelVersion(const ModelVersion& mv)
{
	for (int i = 0; i < mv.surfaces.size(); i++)
	{
		BezierSurfaceC0 bs = *mv.surfaces[i].get();
		surfaces.push_back(make_shared<BezierSurfaceC0>(bs));
	}
}

BezierSurfaceC0* ModelVersion::GetTorso() { return surfaces[0].get(); }
BezierSurfaceC0* ModelVersion::GetLegFront() { return surfaces[2].get(); }
BezierSurfaceC0* ModelVersion::GetLegBack() { return surfaces[3].get(); }
BezierSurfaceC0* ModelVersion::GetTail() { return surfaces[1].get(); }
BezierSurfaceC0* ModelVersion::GetHead() { return surfaces[4].get(); }
BezierSurfaceC0* ModelVersion::GetRightEar() { return surfaces[5].get(); }
BezierSurfaceC0* ModelVersion::GetLeftEar() { return surfaces[6].get(); }
BezierSurfaceC0* ModelVersion::GetBox() { return surfaces[7].get(); }
