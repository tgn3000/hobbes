#include <iostream>
#include <sstream>
#include <hobbes/hobbes.H>
#include "example.H"

using HobbesCharArray = const hobbes::array<char>*;

struct Compiler
{
  static hobbes::cc& instance() {
    static hobbes::cc compiler; // known as cc
    return compiler;
  }

  static void init() {
    if(_isInitialized) return;
    auto& compiler { instance() };
    // compiler.bind("bob", &_bob); ///< binds a Bob object to an instance within the compiler
    _isInitialized = true;
  }

private:  
  static bool _isInitialized;
};

bool Compiler::_isInitialized = false;

DEFINE_STRUCT(employeeInfo,
  (size_t, index),
  (const hobbes::array<char>*, firstName),
  (const hobbes::array<char>*, lastName),
  (const hobbes::array<char>*, loginID)
);

struct employeeRecord {
    void addEmployeeInfo(size_t index, std::string_view firstName, std::string_view lastName, std::string_view loginID) {
        _vEmployeeInfo.push_back(employeeInfo{
            .index = index,
            .firstName = hobbes::makeStringFromStringView(firstName),
            .lastName = hobbes::makeStringFromStringView(lastName),
            .loginID = hobbes::makeStringFromStringView(loginID),
        });
    }

    const hobbes::array<employeeInfo>* generateEmployeesInfo() {
      const auto nEmployees = _vEmployeeInfo.size();
      auto rs = hobbes::makeArray<employeeInfo>(nEmployees);
      auto& data = rs->data;
      for (size_t i = 0; i < nEmployees; ++i) {
        const auto& employeeInfo = _vEmployeeInfo[i];
        auto& datai = data[i];
        datai.index     = employeeInfo.index;
        datai.firstName = employeeInfo.firstName;
        datai.lastName  = employeeInfo.lastName;
        datai.loginID   = employeeInfo.loginID;
      }
      return rs;    
    }

    std::vector<employeeInfo> _vEmployeeInfo;
};


int main() {
    Compiler::init();
    auto& compiler = Compiler::instance();
    compiler.bind("generateEmployeesInfo", memberfn(&employeeRecord::generateEmployeesInfo));

    employeeRecord record;
    compiler.bind("record", &record);

    record.addEmployeeInfo(1, "Tim", "Davidson",  "MCM2001");
    record.addEmployeeInfo(3, "Zed", "Nedialkov", "MS708GZ");
    record.addEmployeeInfo(6, "Kou", "Kanemoto",  "MJN20171107");
    RUN_CODE(
        // std::cout << compiler.compileFn<HobbesCharArray()>("show(record.generateEmployeesInfo())")();
        compiler.compileFn<void()>("print(record.generateEmployeesInfo())")();
    );

    return 0;
}