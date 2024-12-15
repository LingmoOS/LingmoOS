package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
)

const (
	FILE_PATH = "/usr/share/wayland-sessions/LingmoOnXwayland.desktop"
)
var Usage = func(){
  fmt.Println("Please typing as: xlingmo -on or xlingmo -off  ")
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
		if strings.Contains(string(a), "Hioceann") {
                  if enable == "-off"{
				result += "Hioceann=true\n"
			} else {
				result += "Hioceann=false\n"
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
