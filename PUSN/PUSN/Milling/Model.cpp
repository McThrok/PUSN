#include "Model.h"


void Model::LoadElephant(float minZ)
{
	model.clear();

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
				model.push_back(make_shared<BezierSurfaceC0>(BezierSurfaceC0(line)));
			else if (elementName == "tubeC0")
				model.push_back(make_shared<BezierSurfaceC0>(BezierSurfaceC0(line, true)));
		}

	}

	Matrix modelTransform = XMMatrixScaling(4.5f, 4.5f, 9.0f) * XMMatrixTranslation(-5, 0, minZ);

	for (int k = 0; k < model.size(); k++)
	{
		BezierSurfaceC0* surf = model[k].get();
		for (int i = 0; i < surf->GetWidthVertexCount(); i++)
		{
			for (int j = 0; j < surf->GetHeightVertexCount(); j++)
			{
				XMStoreFloat3(&surf->GetVert(i, j), XMVector3TransformCoord(XMLoadFloat3(&surf->GetVert(i, j)), modelTransform));
			}
		}
	}

}
vector<BezierSurfaceC0*> Model::GetSurfaces()
{
	vector<BezierSurfaceC0*> result(8);

	for (int i = 0; i < model.size(); i++)
		result[i] = model[i].get();

	return result;
}

BezierSurfaceC0* Model::GetTorso() {
	return model[0].get();
}
BezierSurfaceC0* Model::GetLegFront() {
	return model[2].get();
}
BezierSurfaceC0* Model::GetLegBack() {
	return model[3].get();
}
BezierSurfaceC0* Model::GetTail() {
	return model[1].get();
}
BezierSurfaceC0* Model::GetHead() {
	return model[4].get();
}
BezierSurfaceC0* Model::GetRightEar() {
	return model[5].get();
}
BezierSurfaceC0* Model::GetLeftEar() {
	return model[6].get();
}
BezierSurfaceC0* Model::GetBox() {
	return model[7].get();
}