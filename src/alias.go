package main

import (
	"encoding/json"
	"errors"
	"log"
	"os"
	"os/exec"
)

type VarsType struct {
	Browser  string `json:"browsers"`
	Terminal string `json:"terminals"`
	Dirs     []struct {
		Alias string `json:"alias"`
		Dir   string `json:"dir"`
	}
}

type AppDataType struct {
	App  string   `json:"app"`
	Args []string `json:"args"`
}

type CommandDataType struct {
	Url     string        `json:"url"`
	Dir     string        `json:"dir"`
	AppData []AppDataType `json:"appData"`
}

type JsonDataType struct {
	Vars    VarsType          `json:"vars"`
	Aliases []AliasObjectType `json:"aliases"`
}
type AliasObjectType struct {
	Alias string `json:"alias"`
	CommandDataType
}

type AliasData struct {
	Browser  string
	Terminal string
	DirMap   map[string]string
	AliasMap map[string]CommandDataType
}

// We are encapsulating the extraction of JSON data so that this memory is freed when we move out of this scope since we do not require the raw JSON data as []bytes or the JsonDataType that we create after the extraction of values from it.
func (aliasData *AliasData) FillFromJson() (err error) {
	const filepath = "commands.json"

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

	aliasData.DirMap = make(map[string]string)
	aliasData.AliasMap = make(map[string]CommandDataType)

	// We are simulating `sets` using `maps`.
	for _, dirObj := range jsonMap.Vars.Dirs {
		aliasData.DirMap[dirObj.Alias] = dirObj.Dir
	}
	for _, aliasObj := range jsonMap.Aliases {
		aliasData.AliasMap[aliasObj.Alias] = CommandDataType{
			Url:     aliasObj.Url,
			Dir:     aliasObj.Dir,
			AppData: aliasObj.AppData,
		}
	}

	return
}

func (aliases *AliasData) ExecuteUrl(aliasUrl string) error {
	if cmd := exec.Command(aliases.Terminal, "start", aliases.Browser, aliasUrl); cmd != nil {
		return cmd.Run()
	} else {
		return errors.New("exec.Command() returned nil.")
	}
}

func (aliases *AliasData) Run(alias string) error {
	commandData, ok := aliases.AliasMap[alias]
	if !ok {
		// TODO: In the future if we don't find a command we will try to find a command that resembles it the most and suggest that.
		return errors.New("Command not found.")
	}

	if commandData.Url != "" {
		return aliases.ExecuteUrl(commandData.Url)
	}
	if commandData.Dir != "" || len(commandData.AppData) != 0 {
		return errors.New("We still have to implement this functionality.")
	}

	return errors.New("Command has no URL.")
}
