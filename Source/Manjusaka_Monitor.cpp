#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <sys/stat.h>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

// 配置文件路径
std::string Manjusaka_conf_path = "/data/media/0/Android/Manjusaka/Manjusaka.conf";

void check_loop(std::string Set_Time, std::string Creen_script, std::string Screen_script) {
    auto check = [&]() {
        std::string Qiule;
        FILE *fp;
        char buffer[1024];
        fp = popen("dumpsys window policy | awk -F= '/mInputRestricted/ {print $2}'", "r");
        fgets(buffer, sizeof(buffer), fp);
        pclose(fp);
        Qiule = buffer;
        Qiule.erase(Qiule.find_last_not_of("\n") + 1);

        static std::string Manjusaka = "false";
        if (Qiule != Manjusaka) {
            Manjusaka = Qiule;
            if (Qiule == "true") {
                std::this_thread::sleep_for(std::chrono::seconds(std::stoi(Set_Time)));
                system(Creen_script.c_str());
            } else {
                system(Screen_script.c_str());
            }
        }
    };
    while (true) {
        check();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    std::string Set_Time, Creen_script, Screen_script;
    {
        // 读取配置文件
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(Manjusaka_conf_path, pt);

        // 获取设置项
        Set_Time = pt.get<std::string>("Settings.Set_Time", "1");
        Creen_script = pt.get<std::string>("Scripts.Creen_script", "/data/off.sh");
        Screen_script = pt.get<std::string>("Scripts.Screen_script", "/data/on.sh");

        // 如果配置文件不存在，则创建并写入默认配置
        std::ofstream outfile;
        struct stat st = {0};
        if (stat("/data/media/0/Android/Manjusaka", &st) == -1) {
            mkdir("/data/media/0/Android/Manjusaka", 0777);
        }
        if (!std::ifstream(Manjusaka_conf_path)) {
            outfile = std::ofstream(Manjusaka_conf_path);
            outfile << "; Author:Manjusaka(酷安@曼珠沙华Y)\n";
            outfile << "; Group:647299031\n";
            outfile << "; QQ:898780441\n";
            outfile << "\n";
            outfile << "[Settings]\n";
            outfile << "; 设置屏幕关闭多长时间执行\n";
            outfile << "Set_Time=1\n";
            outfile << "\n";
            outfile << "[Scripts]\n";
            outfile << "; 屏幕关闭后执行的脚本\n";
            outfile << "Creen_script=/data/off.sh\n";
            outfile << "; 屏幕开启后执行的脚本\n";
            outfile << "Screen_script=/data/on.sh\n";
            outfile.close();
        }
    }

    // 设置配置文件权限为755
    system(("chmod 755 " + Manjusaka_conf_path).c_str());
    std::thread t(check_loop, Set_Time, Creen_script, Screen_script);

    // 减少 CPU 占用率
    std::this_thread::sleep_for(std::chrono::seconds(10));

    t.join();

    return 0;
}
