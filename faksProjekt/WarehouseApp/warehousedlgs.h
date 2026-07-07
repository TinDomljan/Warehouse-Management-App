#pragma once


#ifdef __cplusplus
class QWidget;
#endif


#if defined(_WIN32) || defined(_WIN64)
#  ifdef WAREHOUSEDLGS_EXPORTS
#    define DLGS_API __declspec(dllexport)
#  else
#    define DLGS_API __declspec(dllimport)
#  endif
#else
#  define DLGS_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLGS_API void ShowAboutDialog(QWidget* parent);


DLGS_API void ShowQuickCalcDialog(QWidget* parent);

#ifdef __cplusplus
}
#endif
