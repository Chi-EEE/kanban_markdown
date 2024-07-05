#include <iostream>
#include <md4c.h>

struct KanBan {
    std::string description = "dsa";
};

// Callback for processing block elements
int enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
    KanBan* kanban = static_cast<KanBan*>(userdata);
    switch (type) {
    case MD_BLOCK_DOC:
        std::cout << "[Document Start]\n";
        break;
    case MD_BLOCK_H:
        std::cout << "[Header]\n";
        break;
    case MD_BLOCK_P:
        std::cout << "[Paragraph]\n";
        break;
    case MD_BLOCK_UL:
        std::cout << "[Unordered List]\n";
        break;
    case MD_BLOCK_OL:
        std::cout << "[Ordered List]\n";
        break;
    default:
        std::cout << "Unknown Type: " << type << "\n";
        break;
    }
    return 0;
}

// Callback for leaving block elements
int leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
    KanBan* kanban = static_cast<KanBan*>(userdata);
    switch (type) {
    case MD_BLOCK_DOC:
        std::cout << "[Document End]\n";
        break;
    case MD_BLOCK_H:
        std::cout << "[Header]\n";
        break;
    case MD_BLOCK_P:
        std::cout << "[Paragraph]\n";
        break;
    case MD_BLOCK_UL:
        std::cout << "[Unordered List]\n";
        break;
    case MD_BLOCK_OL:
        std::cout << "[Ordered List]\n";
        break;
    default:
        std::cout << "Unknown Type: " << type << "\n";
        break;
    }
    return 0;
}

// Callback for processing span elements
int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
    KanBan* kanban = static_cast<KanBan*>(userdata);
    switch (type) {
    case MD_SPAN_EM:
        std::cout << "[Emphasis]\n";
        break;
    case MD_SPAN_STRONG:
        std::cout << "[Strong]\n";
        break;
    case MD_SPAN_A:
        std::cout << "[Link]\n";
        break;
    case MD_SPAN_IMG:
        std::cout << "[Image]\n";
        break;
    default:
        break;
    }
    return 0;
}

// Callback for leaving span elements
int leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
    KanBan* kanban = static_cast<KanBan*>(userdata);
    std::cout << "Leaving span type: " << type << std::endl;
    return 0;
}

// Callback for processing text
int text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata) {
    KanBan* kanban = static_cast<KanBan*>(userdata);
    std::string text_content(text, size);
    switch (type) {
    case MD_TEXT_NORMAL:
        std::cout << "Text: " << text_content << "\n";
        break;
    case MD_TEXT_CODE:
        std::cout << "Code: " << text_content << "\n";
        break;
    case MD_TEXT_HTML:
        std::cout << "HTML: " << text_content << "\n";
        break;
    default:
        break;
    }
    return 0;
}


void debug(const char* msg, void* userdata) {
    KanBan* kanban = static_cast<KanBan*>(userdata);
    std::cout << msg << '\n';
}


int main() {
    const std::string test_string = R"(---
Description: 
---

# TODO:

<!-- Kanban-MD:Board -->
## AA

<!-- Kanban-MD:List -->
### To Do:

- [ ] Task  
    - **Description**: sadbfdhubsidfghhausgbifdsiohjaisufjia dfjidsafasdjhksdafbasdnjkls hfnasdkfjladsfndas asndnfjkasdnjkfnaskfnasd
    - **Category**: Bug Report  
    - **Assigned**: Chi-EEE  
    - **Labels**: 
    - **Attachments**:
        -  Test_Image | test_image.txt
    - **Checklist**:
        - [] aaa
    - **Comments**:
        - 
    
- [x] Completed task

<!-- Kanban-MD:List -->
### Doing:

<!-- Kanban-MD:List --> 
### On Hold:

<!-- Kanban-MD:List -->
### Done:
)";

    // Define the parser
    MD_PARSER parser;
    parser.abi_version = 0;
    parser.enter_block = enter_block_callback;
    parser.leave_block = leave_block_callback;
    parser.enter_span = enter_span_callback;
    parser.leave_span = leave_span_callback;
    parser.text = text_callback;
    parser.flags = 0;
    parser.syntax = nullptr;
    parser.debug_log = debug;

    KanBan kanban;
    // Parse the Markdown text
    int result = md_parse(test_string.c_str(), test_string.size(), &parser, &kanban);

    if (result != 0) {
        std::cerr << "Error parsing Markdown text." << std::endl;
    }

    return 0;
}
