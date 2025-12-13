package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
)

func print(jsonMap map[string]any) {
	for key, value := range jsonMap {
		fmt.Printf("%s: %v\n", key, value)
	}
}

func main() {
	filepath := "commands.json"

	jsonBytes, err := os.ReadFile(filepath)
	if err != nil {
		log.Fatalf("Error reading JSON file %s: %v", filepath, err)
	}

	fmt.Printf("%s\n", jsonBytes)
	jsonMap := make(map[string]any)
	if err = json.Unmarshal(jsonBytes, &jsonMap); err != nil {
		log.Fatalf("Error fitting the json data into the jsonMap %v", err)
	}

	print(jsonMap)
}
