#include "ReadDll.hpp"

#include <windows.h>
#include <imagehlp.h>

void getDllFunctions(std::string dllName, std::vector<std::string>& functionList)
{
	DWORD* nameRVAs(0);
	_IMAGE_EXPORT_DIRECTORY* imageExportDirectory;
	unsigned long dirSize;
	_LOADED_IMAGE loadedImage;
	std::string name;
	if (MapAndLoad(dllName.c_str(), NULL, &loadedImage, TRUE, TRUE))
	{
		imageExportDirectory = (_IMAGE_EXPORT_DIRECTORY*)ImageDirectoryEntryToData(
			loadedImage.MappedAddress, false, IMAGE_DIRECTORY_ENTRY_EXPORT, &dirSize);
		if (imageExportDirectory != NULL)
		{
			nameRVAs = (DWORD*)ImageRvaToVa(loadedImage.FileHeader, loadedImage.MappedAddress,
											 imageExportDirectory->AddressOfNames, NULL);
			for (size_t i = 0; i < imageExportDirectory->NumberOfNames; i++)
			{
				name = (char*)ImageRvaToVa(loadedImage.FileHeader, loadedImage.MappedAddress, nameRVAs[i], NULL);
				functionList.push_back(name);
			}
		}
		UnMapAndLoad(&loadedImage);
	}
}
