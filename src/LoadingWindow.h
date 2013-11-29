#ifndef LOADINGW_H_
#define LOADINGW_H_

void showLoadingWindow(Layer* otherLayer);
void hideLoadingWindow(Layer* otherLayer);
void setLoadingWindowStatus(char* status);
void initLoadingWindow(Layer* topLayer);

#endif /* LOADINGW_H_ */