package main

import (
	"encoding/json"
	"log"
	"os"
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

type JsonDataType struct {
	Vars    VarsType          `json:"vars"`
	Aliases []AliasObjectType `json:"aliases"`
}

// We are encapsulating the extraction of JSON data so that this memory is freed when we move out of this scope since we do not require the raw JSON data as []bytes or the JsonDataType that we create after the extraction of values from it.
func ExtractFromJson() (
	browsers map[string]bool,
	terminals map[string]bool,
	dirs map[string]string,
	aliases AliasMap,
) {
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

	// We still have to initialise them even though we mentioned them as the return values
	browsers = make(map[string]bool)
	terminals = make(map[string]bool)
	dirs = make(map[string]string)
	aliases = make(AliasMap)

	// We are simulating `sets` using `maps`.
	for _, browser := range jsonMap.Vars.Browsers {
		browsers[browser] = true
	}
	for _, terminal := range jsonMap.Vars.Terminals {
		terminals[terminal] = true
	}
	for _, dirObj := range jsonMap.Vars.Dirs {
		dirs[dirObj.Alias] = dirObj.Dir
	}
	for _, aliasObj := range jsonMap.Aliases {
		aliases[aliasObj.Alias] = CommandDataType{
			Url:     aliasObj.Url,
			Dir:     aliasObj.Dir,
			AppData: aliasObj.AppData,
		}
	}

	return
}
