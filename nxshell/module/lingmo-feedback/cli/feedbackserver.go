package main

import (
    "fmt"
    "net/http"
    "os"
)

func ServeFeedbackFileName(w http.ResponseWriter, r *http.Request) {
    r.ParseForm()
    w.Header().Set("Access-Control-Allow-Origin", "*")
    feedback_name := os.Args[1]
    fmt.Fprintf(w, feedback_name)
}

func ServeFeedbackFile(w http.ResponseWriter, r *http.Request){
    w.Header().Set("Access-Control-Allow-Origin", "*")
    name := os.Args[1]
    http.ServeFile(w, r, name)
}

func main() {
    http.HandleFunc("/", ServeFeedbackFileName)
    if os.Args[1] != "generating" {
        name_route := "/" + os.Args[1]
        http.HandleFunc(name_route, ServeFeedbackFile)
    }
    http.ListenAndServe(":9090", nil)
}
