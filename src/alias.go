package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"os"
)

type VarsType struct {
	Browser  string `json:"browser"`
	Terminal string `json:"terminal"`
	Editor   string `json:"editor"`
	Dirs     []struct {
		Alias string `json:"alias"`
		Dir   string `json:"dir"`
	}
}

type CommandType struct {
	Tool string   `json:"tool"`
	Args []string `json:"args"`
}

type CommandDataType struct {
	Description string        `json:"description"`
	Commands    []CommandType `json:"commands"`
}

type AliasObjectType struct {
	Alias string `json:"alias"`
	CommandDataType
}

type JsonDataType struct {
	Vars    VarsType          `json:"vars"`
	Aliases []AliasObjectType `json:"aliases"`
}

type AliasData struct {
	Browser  string
	Terminal string
	Editor   string
	DirMap   map[string]string
	AliasMap map[string]CommandDataType
}

// We are encapsulating the extraction of JSON data so that this memory is freed when we move out of this scope since we do not require the raw JSON data as []bytes or the JsonDataType that we create after the extraction of values from it.
func (aliasData *AliasData) FillFromJson(filepath string) (err error) {
	jsonBytes, err := os.ReadFile(filepath)
	if err != nil {
		log.Fatalf("Error reading JSON file %s: %v", filepath, err)
		return
	}

	var jsonMap JsonDataType
	if err = json.Unmarshal(jsonBytes, &jsonMap); err != nil {
		log.Fatalf("Error fitting the json data into the jsonMap %v", err)
		return
	}

	// We still have to initialise them even though we mentioned them as the return values
	aliasData.Browser = jsonMap.Vars.Browser
	aliasData.Terminal = jsonMap.Vars.Terminal
	aliasData.Editor = jsonMap.Vars.Editor

	aliasData.DirMap = make(map[string]string)
	aliasData.AliasMap = make(map[string]CommandDataType)

	// We are simulating `sets` using `maps`.
	for _, dirObj := range jsonMap.Vars.Dirs {
		aliasData.DirMap[dirObj.Alias] = dirObj.Dir
	}
	for _, aliasObj := range jsonMap.Aliases {
		aliasData.AliasMap[aliasObj.Alias] = CommandDataType{
			Description: aliasObj.Description,
			Commands:    aliasObj.Commands,
		}
	}

	return
}

func (aliasData *AliasData) String() (output string) {
	output = fmt.Sprintf("Browser: %s\nTerminal: %s\nEditor: %s\n", aliasData.Browser, aliasData.Terminal, aliasData.Editor)

	output = fmt.Sprintln(output, "DirMap:")
	for key, value := range aliasData.DirMap {
		output = fmt.Sprintln(output, key, value)
	}

	fmt.Println("AliasMap:")
	for key, value := range aliasData.AliasMap {
		output = fmt.Sprintf("%s%v\n%v\n", output, key, value)
	}

	return
}

func (aliases *AliasData) Run(alias string) error {
	commandData, ok := aliases.AliasMap[alias]
	if !ok {
		// TODO: In the future if we don't find a command we will try to find a command that resembles it the most and suggest that.
		return errors.New("Command not found.")
	}
	if len(commandData.Commands) == 0 {
		return errors.New("There is not set of commands provided.")
	}

	return nil

	commandLine := []string{"git-bash"}

	for index, command := range commandData.Commands {
		var currArgs []string

		if index != 0 {
			// Chaining command so that they happens as part of one session.
			currArgs = append(currArgs, "&&")
		}
		if command.Tool != "cd" {
			currArgs = append(currArgs, "start")
		}

		currArgs = append(currArgs, append([]string{command.Tool}, command.Args...)...)
		commandLine = append(commandLine, currArgs...)
	}

	return errors.New("Command has no URL.")
}
