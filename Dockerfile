FROM ubuntu
ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools build-essential -y
WORKDIR /root/server/
COPY *.cpp *.h *.pro ./
RUN qmake HappyMeals.pro && make
ENTRYPOINT ["./HappyMeals"]
