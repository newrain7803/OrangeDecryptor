// DatDecryptor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DatDecryptor.h"

#include <fstream>
#include <algorithm>
#include "Dispatcher.hpp"

#define PROGRAM_VER_STR "1.2"

enum DecryptMode
{
	AutoDetect,
	DataDecryption,
	UnzipAndDataDecryption,
	WavExtraction,
	OggExtraction
};

const wchar_t* GetDecryptMode(DecryptMode mode)
{
	switch(mode)
	{
	case DataDecryption:
		return L"DAT Decryption / DAT 解密";

	case UnzipAndDataDecryption:
		return L"Unzip + DAT Decryption / 解压 + DAT 解密";

	case OggExtraction:
		return L"ogg extraction";

	case WavExtraction:
		return L"wav extraction";

	case AutoDetect:
		return L"auto detect / 自动检测";

	default:
		return L"<unknown - 未知>";
	}
}

int wmain(int argc, wchar_t *argv[])
{
	printf("100%% Orange Pak decryptor v" PROGRAM_VER_STR "\n");
	printf("by: Jixun Moe<https://jixun.moe/>\n");
	printf("\n");
	printf("This tool is provided \"AS IS\", without warranty of any kind,\n");
	printf("  and commercial usage is strictly prohibited.\n");
	printf("\n");
	wprintf(L"该工具不提供任何承诺；同时禁止将该工具用于商业用途。\n");
	printf("\n");

	if (argc < 3)
	{
		printf("Usage / 参数:\n");
		printf("  OrangeDecryptor <input> <output> [mode]\n");
		printf("  OrangeDecryptor <输入文件> <输出文件/目录> [模式]\n");
		printf("\n");

		printf(
			"mode is the decryption mode; it can be one of the following: \n"
			"  dat      Apply dat decryption. Note: Any file not recognised will fallback to this mode.\n"
			"  zip      Extract file as zip to output(as directory), then apply dat decryption\n"
			"             (and add extension if possible) if the file name end with \".dat\".\n"
			"  ogg      Seperate packed audio file to individual files; name pattern: [output]/[000].ogg\n"
			"  wav      Extract WAV sound (Sound Effects & Voice) from pak archive.\n"
			"  auto     Default; it will check the file header and perform action.\n"
		);
		wprintf(
			L"[模式] 为解密模式，留空则自动尝试探测，否则可以指定下述的值之一: \n"
			L"  dat      尝试通用 dat 格式解密。如果自动探测失败将自动回退到这个模式。\n"
			L"  zip      解压输入文件到指定的目录。如果文件名为 .dat 结尾则尝试使用 dat 格式解密，并尝试添加认识的后缀名。\n"
			L"  ogg      分离多个 ogg 打包的文件。输出名称格式: [output]/[000].ogg\n"
			L"  wav      从 pak 包提取 WAV 文件 (音效 + 语音)。\n"
			L"  auto     默认模式；将尝试识别输入文件所使用的加密或打包方法。\n"
		);
		printf("\n");
		return 0;
	}

	DecryptMode decryptMode = AutoDetect;
	if (argc > 3)
	{
		std::wstring mode(argv[3]);
		std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

		if (mode == L"dat")
		{
			decryptMode = DataDecryption;
		} else if (mode == L"zip")
		{
			decryptMode = UnzipAndDataDecryption;
		}
		else if (mode == L"wav")
		{
			decryptMode = WavExtraction;
		}
		else if (mode == L"ogg")
		{
			decryptMode = OggExtraction;
		}
	}

	TCHAR* input = argv[1];
	TCHAR* output = argv[2];

	if (decryptMode == AutoDetect)
	{
		auto f = File::CreateReader(input);
		const auto magic = f->Read<DWORD>();

		if (magic == OGG_MAGIC_OGGS)
		{
			decryptMode = OggExtraction;
		} else if (LOWORD(magic) == 'KP')
		{
			// 大写的 PK，是 ZIP 压缩文件
			decryptMode = UnzipAndDataDecryption;
		} else
		{
			// 看看是不是 WAV + 自制压缩
			// 格式是 数据大小 + 文件名 + 0x12

			std::string name;
			f->ReadNullTerminated(name);
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);

			// 文件名以 .wav 結尾，或之後的 Flag 是 0x12
			if (HasEnding<std::string>(name, ".wav") || f->Read<uint32_t>() == 0x12)
			{
				decryptMode = WavExtraction;
			} else
			{
				// 如果不知道是什么格式
				// 进行数据解密操作
				decryptMode = DataDecryption;
			}
		}

		wprintf(L"Decryption mode (auto detected): %s", GetDecryptMode(decryptMode));

		f->Close();
	} else
	{
		wprintf(L"Decryption mode: %s", GetDecryptMode(decryptMode));
	}

	printf("\n");

	switch(decryptMode)
	{
	case DataDecryption:
		DoDataDecryption(input, output);
		break;

	case UnzipAndDataDecryption:
		DoUnzipAndDataDecryption(input, output);
		break;

	case OggExtraction:
		DoOggExtraction(input, output);
		break;

	case WavExtraction:
		DoWavExtraction(input, output);
		break;

	default:
		break;
	}
    return 0;
}

