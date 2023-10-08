#!/bin/bash

# Function to get the language from environment variables
get_language() {
    if [[ ! -z "$LANG" ]]; then
        echo "$LANG"
    elif [[ ! -z "$LANGUAGE" ]]; then
        echo "$LANGUAGE" | awk -F: '{print $1}'
    elif [[ ! -z "$LC_ALL" ]]; then
        echo "$LC_ALL"
    else
        echo "en_US.UTF-8"  # Default to English (United States) if no language is set
    fi
}
