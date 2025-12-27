package main

import (
	"log"
	"os"
)

func main() {
	// Check if we need to run the program or not.
	if len(os.Args) < 2 {
		// This means there is only one argument on the command line which is this program itself.
		log.Fatal("No arguments provided.")
		os.Exit(1)
		return
	} else if len(os.Args) > 2 {
		log.Fatal("This program only takes one argument which is an alias")
		os.Exit(1)
		return
	}

	// Load the commands.
	var aliasData AliasData
	if err := aliasData.FillFromJson("commands.json"); err != nil {
		log.Fatal(err)
		os.Exit(1)
		return
	}

	// Run the command.
	// alias := os.Args[1]
	//
	// if err := aliasData.Run(alias); err != nil {
	// 	log.Fatal(err)
	// 	os.Exit(1)
	// 	return
	// }

	// for alias, _ := range aliasData.AliasMap {
	//
	// }
}
