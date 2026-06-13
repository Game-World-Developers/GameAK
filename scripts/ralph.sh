#!/usr/bin/env bash

# This scripts run a LLM loop called Ralph Wiggum

PROMPT_FILE="PROMPT.md"
MAX_ITERATIONS=50
CURRENT_ITERATIONS=0

# Check if we are not root
if [[ "$EUID" -eq 0 ]]; then
  echo "Please do not run this script as root. Exiting."
  exit 1
fi 

# Check if opencode is installed
if ! command -v opencode &> /dev/null; then
  echo "Opencode could not be found. Please install it and try again."
  exit 1
fi

# Check if node is installed
if ! command -v node &> /dev/null; then
  echo "Node.js could not be found. Please install it and try again."
  exit 1
fi

# Check if the prompt file exists
if [[ ! -f "$PROMPT_FILE" ]]; then
  echo "PROMPT.md not found. Please create one with the initial prompt fopr ralph wiggum"

  exit 1
fi

while [[ $CURRENT_ITERATIONS -lt $MAX_ITERATIONS ]]; do 
  # Read the prompt file
  
  opencode run --dangerously-skip-permissions < "$PROMPT_FILE" | tee output.txt

  if grep -q "<promise>DONE</promise>" output.txt; then
    echo "✔️Ralph Wiggum has completed his task. Exiting loop."
    break
  fi

  CURRENT_ITERATIONS=$((CURRENT_ITERATIONS + 1))
done

echo "Ralph Wiggum loop has ended after $CURRENT_ITERATIONS iterations."
exit 1
