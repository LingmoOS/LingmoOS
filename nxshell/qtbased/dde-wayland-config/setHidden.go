package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
)

const (
	FILE_PATH = "/usr/share/wayland-sessions/DeepinOnXwayland.desktop"
)
var Usage = func(){
  fmt.Println("Please typing as: xdeepin -on or xdeepin -off  ")
}
func main() {
        args := os.Args
        if args == nil || len(args) <2 {
            Usage()
            return
        }
        enable := args[1]
        if enable != "-on" && enable != "-off"{
            Usage()
            return
        }
	file, err := os.OpenFile(FILE_PATH, os.O_RDWR, 0666)
	if err != nil {
		panic(err)
	}

	reader := bufio.NewReader(file)
	var result = ""
	for {
		a, _, c := reader.ReadLine()
		if c == io.EOF {
			break
		}
		//fmt.Println(string(a))
		if strings.Contains(string(a), "Hidden") {
                  if enable == "-off"{
				result += "Hidden=true\n"
			} else {
				result += "Hidden=false\n"
			}
		} else {
			result += string(a) + "\n"
		}
	}
	file.Close()
	file, err = os.OpenFile(FILE_PATH, os.O_RDWR|os.O_TRUNC, 0666)
	w := bufio.NewWriter(file)

	w.WriteString(result)

	if err != nil {

		panic(err)

	}

	w.Flush()
	file.Close()
}
