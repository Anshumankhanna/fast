package main

import (
	"errors"
	"fmt"
	"log"
	"os"
	"os/exec"
)

type AliasMap map[string]CommandDataType

func (aliases AliasMap) Run(alias string) (*exec.Cmd, error) {
	commandData, ok := aliases[alias]
	if !ok {
		return nil, errors.New("Command not found.")
	}
	if commandData.Url != nil {
		return exec.Command("powershell", "start", "brave", *commandData.Url), nil
	}

	return nil, errors.New("Command has no URL.")
}

func main() {
	_, _, _, aliases := ExtractFromJson()

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
