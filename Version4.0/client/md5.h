#include "function.h"

int get_file_md5(const std::string &file_name, std::string &md5_value)
{
	md5_value.clear();

	std::ifstream file(file_name.c_str(), std::ifstream::binary);
	if (!file)
	{
		return -1;
	}

	MD5_CTX md5Context;
	MD5_Init(&md5Context);

	char buf[1024 * 16];
	while (file.good())
	{
		file.read(buf, sizeof(buf));
		MD5_Update(&md5Context, buf, file.gcount());
	}

	unsigned char result[MD5_DIGEST_LENGTH];
	MD5_Final(result, &md5Context);

	char hex[35];
	memset(hex, 0, sizeof(hex));
	for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
	{
		sprintf(hex + i * 2, "%02x", result[i]);
	}
	hex[32] = '\0';
	md5_value = string(hex);

	return 0;
}


// int main(int argc, char *argv[])
// {
// 	string file_name = "./Netdisk/Version3.0/client/file";
// 	string md5value;
// 	int ret = get_file_md5(file_name, md5value);
// 	if (ret < 0)
// 	{
// 		printf("get file md5 failed. file=%s\n", file_name.c_str());
// 		return -1;
// 	}
// 	printf("the md5value=%s\n", md5value.c_str());
// }