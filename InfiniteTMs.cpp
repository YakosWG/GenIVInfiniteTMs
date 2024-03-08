#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>

static bool replace_bytes_in_place(const std::string& arm9_path,
    std::size_t offset, const std::vector<unsigned char>& old_data,
    const std::vector<unsigned char>& new_data) {

    std::fstream arm9(arm9_path, std::ios::binary | std::ios::in | std::ios::out);

    if (!arm9.is_open()) {
        std::cerr << "Error opening file: " << arm9_path << std::endl;
        return false;
    }

    arm9.seekg(0, std::ios::end);
    std::size_t file_size = arm9.tellg();
    arm9.seekg(0, std::ios::beg);

    //should never occur if real rom is used
    if (offset > file_size) {
        std::cerr << "Error: Offset is larger than file size." << std::endl;
        return false;
    }

    std::vector<unsigned char> data(file_size);

    arm9.read(reinterpret_cast<char*>(data.data()), file_size);


    //If for some reason a non Pokémon ROM file is both long enough and has the right bytes at the right offset this will fry it
    if (data[offset] == old_data[0]) {

        std::copy(new_data.begin(), new_data.end(), data.begin() + offset);
        arm9.seekp(0, std::ios::beg);
        arm9.write(reinterpret_cast<const char*>(data.data()), file_size);

    }
    else if (data[offset] == new_data[0]) {
        std::cerr << "Warning: Byte already set." << std::endl;
    }
    else {
        std::cerr << "Warning: Unexpected Byte found at specified offset." << std::endl << "Found Byte: " << std::hex << static_cast<int>(data[offset]) << std::endl
        << "Make sure you are using the right ROM (HGSS / Platinum US)" << std::endl;
        return false;
    }

    arm9.close();
    return true;
}

static bool runNDSTool(std::string rom_name, bool extract) {

    std::string run_nds_tool = "";
    
    if (extract)
        run_nds_tool = "..\\ndstool.exe -x ..\\" + rom_name + " -9 arm9.bin -7 arm7.bin -y9 y9.bin -y7 y7.bin -d data -y overlay -t banner.bin -h header.bin";
    else
        run_nds_tool = "..\\ndstool.exe -c ..\\" + rom_name + " -9 arm9.bin -7 arm7.bin -y9 y9.bin -y7 y7.bin -d data -y overlay -t banner.bin -h header.bin";

    //this line of code will 100% flag the exe as a virus
    int result = system(run_nds_tool.c_str());

    if (result == -1) {
        std::cerr << "Error: Could not execute ndstool.exe" << std::endl;
        return false;
    }
    else if (result > 0) {
        std::cerr << "Error: ndstool.exe exited with code " << result << std::endl;
        return false;
    }

    return true;

}


int main() {

    std::string rom_name;
    std::string rom_type;

    std::cout << "Enter name of ROM: " << std::endl;
    std::cin >> rom_name;

    std::cout << "Enter ROM Type (hgss / pt): " << std::endl;
    std::cin >> rom_type;

    //Make TMs reusable

    std::size_t offset;

    if (rom_type == "hgss") 
    {
        offset = 0x825A7;
    } 
    else if (rom_type == "pt") 
    {
        offset = 0x865EB;
    }
    else
    {
        std::cerr << rom_type << " is not a valid rom type!";
        return 1;
    }

    std::vector<unsigned char> old_data = { 0xD1 };  // Byte to be replaced (D1 in hex)
    std::vector<unsigned char> new_data = { 0xE0 };  // New byte to write (E0 in hex)

    std::string sub_folder_name = rom_name + "_contents";

    std::filesystem::create_directory(sub_folder_name);
    std::filesystem::current_path(sub_folder_name);

    if (!runNDSTool(rom_name, true)) {
        return 1;
    }

    std::cout << "ndstool.exe finished successfully." << std::endl;

    std::string arm9_path = "arm9.bin";

    if (replace_bytes_in_place(arm9_path, offset, old_data, new_data)) {
        std::cout << "Modification at offset " << std::hex << offset << " successful." << std::endl;
    }
    else {
        std::cerr << "Modification at offset " << std::hex << offset << " failed." << std::endl;
    }

    //Make HMs deletable

    if (rom_type == "hgss")
    {
        offset = 0x78034;
    }
    else if (rom_type == "pt")
    {
        offset = 0x7D29C;
    }   
    
    old_data = { 0x01 };  // Byte to be replaced (01 in hex)
    new_data = { 0x00 };  // New byte to write (00 in hex)

    if (replace_bytes_in_place(arm9_path, offset, old_data, new_data)) {
        std::cout << "Modification at offset " << offset << " successful." << std::endl;
    }
    else {
        std::cerr << "Modification at offset " << offset << " failed." << std::endl;
    }

    if (!runNDSTool(rom_name, false)) {
        return 1;
    }

    return 0;
}