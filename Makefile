CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

SRC = main.cpp filehandler.cpp student_ops.cpp course_ops.cpp attendance.cpp grades.cpp fee_tracker.cpp reports.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = campus_engine

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
