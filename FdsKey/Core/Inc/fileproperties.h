#ifndef INC_FILEPROPERTIES_H_
#define INC_FILEPROPERTIES_H_

#include "ff.h"

#define FILE_PROPERTIES_WRITE_PROTECT 0
#define FILE_PROPERTIES_RESTORE_BACKUP 1
#define FILE_PROPERTIES_DELETE 2

void file_properties(char *directory, FILINFO *fno);

#endif /* INC_FILEPROPERTIES_H_ */
