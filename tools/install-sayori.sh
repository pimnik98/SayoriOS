#!/bin/bash

file_path="/etc/grub.d/40_custom"

# Function to display messages based on the language
display_message() {
    case $1 in
        "en" )
            case $2 in
                "file_exists" )
                    echo "File $file_path exists."
                    ;;
                "replace_confirmation" )
                    read -p "Replace it? (Y/N): " answer
                    ;;
                "replace_file" )
                    echo "Replacing the file..."
                    ;;
                "no_replace" )
                    echo "No replacement necessary."
                    ;;
                "invalid_input" )
                    echo "Invalid input. No replacement will be performed."
                    ;;
                "file_not_exist" )
                    echo "File $file_path does not exist."
                    ;;
                *)
                    echo "Unknown message key: $2"
                    ;;
            esac
            ;;
        "ru" )
            case $2 in
                "file_exists" )
                    echo "Файл $file_path существует."
                    ;;
                "replace_confirmation" )
                    read -p "Заменить его? (Д/Н): " answer
                    ;;
                "replace_file" )
                    echo "Замена файла..."
                    ;;
                "no_replace" )
                    echo "Замена не требуется."
                    ;;
                "invalid_input" )
                    echo "Некорректный ввод. Замена не будет выполнена."
                    ;;
                "file_not_exist" )
                    echo "Файл $file_path не существует."
                    ;;
                *)
                    echo "Неизвестный ключ сообщения: $2"
                    ;;
            esac
            ;;
        "sv" )
            case $2 in
                "file_exists" )
                    echo "Filen $file_path finns."
                    ;;
                "replace_confirmation" )
                    read -p "Vill du ersätta den? (J/N): " answer
                    ;;
                "replace_file" )
                    echo "Ersätter filen..."
                    ;;
                "no_replace" )
                    echo "Inget byte behövs."
                    ;;
                "invalid_input" )
                    echo "Ogiltig inmatning. Ingen ersättning kommer att utföras."
                    ;;
                "file_not_exist" )
                    echo "Filen $file_path finns inte."
                    ;;
                *)
                    echo "Okänt meddelande: $2"
                    ;;
            esac
            ;;
        *)
            echo "Unknown language code: $1"
            ;;
    esac
}

# Check if the file exists
if [ -f "$file_path" ]; then
    display_message "en" "file_exists"
    display_message "en" "replace_confirmation"
    case $answer in
        [Yy]* )
            display_message "en" "replace_file"
            cp 40_custom $file_path
            # Add your code to replace the file
            ;;
        [Nn]* )
            display_message "en" "no_replace"
            ;;
        * )
            display_message "en" "invalid_input"
            ;;
    esac
else
    display_message "en" "file_not_exist"
fi
