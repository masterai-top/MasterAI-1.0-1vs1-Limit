#ifndef _FILES_H_
#define _FILES_H_

#include <string>

class Files {
public:
  static void Init(const std::string &cfr_dir,const std::string &static_dir);
  static const char *CFRBase(void);
  static const char *StaticBase(void);
private:
  static std::string cfr_base_;
  static std::string static_base_;
};

#endif
