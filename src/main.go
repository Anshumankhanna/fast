package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"os"
	"os/exec"
)

type VarsType struct {
	Browsers  []string `json:"browsers"`
	Terminals []string `json:"terminals"`
	Dirs      []struct {
		Alias string `json:"alias"`
		Dir   string `json:"dir"`
	}
}

type AppDataType struct {
	App  string    `json:"app"`
	Args *[]string `json:"args"`
}

type CommandDataType struct {
	Url     *string        `json:"url"`
	Dir     *string        `json:"dir"`
	AppData *[]AppDataType `json:"appData"`
}

type AliasObjectType struct {
	Alias string `json:"alias"`
	CommandDataType
}

type AliasMap map[string]CommandDataType

type JsonDataType struct {
	Vars    VarsType          `json:"vars"`
	Aliases []AliasObjectType `json:"aliases"`
}

func (aliases AliasMap) Run(alias string) (*exec.Cmd, error) {
	commandData, ok := aliases[alias]
	if !ok {
		return nil, errors.New("Command not found.")
	}
	if commandData.Url != nil {
		return exec.Command("cmd", "/C", "start", "brave", *commandData.Url), nil
	}

	return nil, errors.New("Command has no URL.")
}

func main() {
	const filepath = "commands.json"

	jsonBytes, err := os.ReadFile(filepath)
	if err != nil {
		log.Fatalf("Error reading JSON file %s: %v", filepath, err)
		os.Exit(1)
	}

	var jsonMap JsonDataType
	if err := json.Unmarshal(jsonBytes, &jsonMap); err != nil {
		log.Fatalf("Error fitting the json data into the jsonMap %v", err)
		os.Exit(1)
	}

	// We are simulating `sets` using `maps`.
	browsers := make(map[string]bool)
	for _, browser := range jsonMap.Vars.Browsers {
		browsers[browser] = true
	}

	terminals := make(map[string]bool)
	for _, terminal := range jsonMap.Vars.Terminals {
		terminals[terminal] = true
	}

	dirs := make(map[string]string)
	for _, dirObj := range jsonMap.Vars.Dirs {
		dirs[dirObj.Alias] = dirObj.Dir
	}

	aliases := make(AliasMap)
	for _, aliasObj := range jsonMap.Aliases {
		aliases[aliasObj.Alias] = CommandDataType{
			Url:     aliasObj.Url,
			Dir:     aliasObj.Dir,
			AppData: aliasObj.AppData,
		}
	}

	const urlAlias = "lc"

	if cmd, err := aliases.Run(urlAlias); err != nil {
		log.Fatalf("Error creating the command. Error: %s\n", err)
		os.Exit(1)
	} else if cmd == nil {
		log.Fatalf("Didn't get command.\n")
		os.Exit(1)
	} else if err := cmd.Run(); err != nil {
		log.Fatalf("Error running the command. Error: %s\n", err)
		os.Exit(1)
	} else {
		fmt.Print("Command ran successfully.")
	}
}
