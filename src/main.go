package main

import (
	"log"
	"os"
)

func main() {
	if argc := len(os.Args); argc < 2 {
		log.Fatal("No arguments provided.")
		os.Exit(1)
	} else if argc > 2 {
		log.Fatal("This program only takes one argument which is an alias")
		os.Exit(1)
	}

	var aliasData AliasData

	if err := aliasData.FillFromJson(); err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
	if err := aliasData.Run(os.Args[1]); err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
}
