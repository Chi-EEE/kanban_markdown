document.getElementById('add-list').addEventListener('click', function () {
    const board = document.getElementById('board');

    const newList = document.createElement('div');
    newList.className = 'list';

    const listTitle = document.createElement('input');
    listTitle.className = 'list-title';
    listTitle.placeholder = 'Enter list title';

    // Track focus on listTitle to handle automatic removal
    let isListTitleChanged = false; // Flag to track if list title is changed
    listTitle.addEventListener('input', function () {
        isListTitleChanged = true;
    });

    listTitle.addEventListener('blur', function () {
        if (!isListTitleChanged) {
            newList.remove(); // Remove the list if title is unchanged
        }
    });

    const cards = document.createElement('div');
    cards.className = 'cards';

    const addCardButton = document.createElement('button');
    addCardButton.className = 'add-card';
    addCardButton.innerText = 'Add another card +';
    addCardButton.addEventListener('click', function () {
        const card = document.createElement('div');
        card.className = 'card';

        const cardTitleInput = document.createElement('input');
        cardTitleInput.className = 'card-title';
        cardTitleInput.placeholder = 'Enter card title';

        // Track focus on cardTitleInput to handle automatic removal
        let isCardTitleChanged = false; // Flag to track if card title is changed
        cardTitleInput.addEventListener('input', function () {
            isCardTitleChanged = true;
        });

        cardTitleInput.addEventListener('blur', function () {
            if (!isCardTitleChanged) {
                card.remove(); // Remove the card if title is unchanged
            } else {
                cardTitleInput.readOnly = true; // Make the card title input read-only
            }
        });

        const cardMenuButton = document.createElement('button');
        cardMenuButton.className = 'card-menu';
        cardMenuButton.innerText = '⋮';
        cardMenuButton.addEventListener('click', function (event) {
            event.stopPropagation();
            closeAllMenus(); // Close all other open menus before opening this one
            const actionsMenu = card.querySelector('.card-menu-actions');
            actionsMenu.style.display = actionsMenu.style.display === 'block' ? 'none' : 'block';
        });

        const cardMenuActions = document.createElement('div');
        cardMenuActions.className = 'card-menu-actions';
        cardMenuActions.style.display = 'none';

        const editCardButton = document.createElement('button');
        editCardButton.className = 'edit-card';
        editCardButton.innerText = 'Edit';
        editCardButton.addEventListener('click', function (event) {
            event.stopPropagation();
            document.getElementById('edit-card-title').value = cardTitleInput.value;
            document.getElementById('edit-card-description').value = ''; // Clear previous description
            document.getElementById('card-modal').style.display = 'block';
            cardTitleInput.readOnly = false;
            cardTitleInput.focus();

            // Save changes when modal is closed
            document.getElementById('save-card').onclick = function () {
                cardTitleInput.value = document.getElementById('edit-card-title').value;
                // Update card description if needed
                // const cardDescription = document.getElementById('edit-card-description').value;
                // Update card data here
                document.getElementById('card-modal').style.display = 'none';
            };
        });

        cardMenuActions.appendChild(editCardButton);

        card.appendChild(cardTitleInput);
        card.appendChild(cardMenuButton);
        card.appendChild(cardMenuActions);
        cards.appendChild(card);

        // Open modal on card click
        card.addEventListener('click', function () {
            if (isCardTitleChanged) {
                document.getElementById('edit-card-title').value = cardTitleInput.value;
                // Optionally, update description in modal
                document.getElementById('edit-card-description').value = ''; // Clear previous description
                document.getElementById('card-modal').style.display = 'block';
                document.getElementById('save-card').onclick = function () {
                    cardTitleInput.value = document.getElementById('edit-card-title').value;
                    document.getElementById('card-modal').style.display = 'none';
                };
            }
        });

        // Focus on the card title input after it's added to ensure prompt is shown
        cardTitleInput.focus();
    });

    const listActionsButton = document.createElement('button');
    listActionsButton.className = 'list-actions';
    listActionsButton.innerText = '⋮';
    listActionsButton.addEventListener('click', function (event) {
        event.stopPropagation();
        closeAllMenus(); // Close all other open menus before opening this one
        const listActionsMenu = newList.querySelector('.list-actions-menu');
        listActionsMenu.style.display = listActionsMenu.style.display === 'block' ? 'none' : 'block';
    });

    const listActionsMenu = document.createElement('div');
    listActionsMenu.className = 'list-actions-menu';
    listActionsMenu.style.display = 'none';

    const removeListButton = document.createElement('button');
    removeListButton.className = 'remove-list';
    removeListButton.innerText = 'Remove list';
    removeListButton.addEventListener('click', function () {
        newList.remove();
    });

    listActionsMenu.appendChild(removeListButton);

    newList.appendChild(listTitle);
    newList.appendChild(listActionsButton);
    newList.appendChild(listActionsMenu);
    newList.appendChild(cards); // Append cards before addCardButton
    newList.appendChild(addCardButton); // Append addCardButton after cards

    board.insertBefore(newList, document.getElementById('add-list'));

    // Focus on the list title input after it's added to ensure prompt is shown
    listTitle.focus();
});

// Background color picker
document.getElementById('background-color-picker').addEventListener('input', function (event) {
    document.body.style.backgroundColor = event.target.value;
});

// Modal functionality
const modal = document.getElementById('card-modal');
const span = document.getElementsByClassName('close')[0];

span.onclick = function () {
    modal.style.display = 'none';
};

window.onclick = function (event) {
    if (event.target == modal) {
        modal.style.display = 'none';
    }
    closeAllMenus(); // Close all menus if clicking outside of them
};

// Function to close all open menus
function closeAllMenus() {
    const allMenus = document.querySelectorAll('.card-menu-actions, .list-actions-menu');
    allMenus.forEach(function (menu) {
        menu.style.display = 'none';
    });
}

// Save card button in modal
document.getElementById('save-card').addEventListener('click', function () {
    const cardTitle = document.getElementById('edit-card-title').value;
    const cardDescription = document.getElementById('edit-card-description').value;

    // Find the correct card and update its title and description
    const allCards = document.querySelectorAll('.card');
    allCards.forEach(function (card) {
        const cardTitleInput = card.querySelector('.card-title');
        if (cardTitleInput.value === cardTitle) {
            // Update card title
            cardTitleInput.value = cardTitle;

            // Update card description (if needed)
            // const cardDescriptionElement = card.querySelector('.card-description');
            // cardDescriptionElement.textContent = cardDescription;
        }
    });

    // Find the correct list and update its title
    const allLists = document.querySelectorAll('.list');
    allLists.forEach(function (list) {
        const listTitleInput = list.querySelector('.list-title');
        if (listTitleInput.value === cardTitle) {
            // Update list title
            listTitleInput.value = cardTitle;
        }
    });

    modal.style.display = 'none';
});

// Get elements
const kanbanTitle = document.getElementById('kanban-title');
const editTitleInput = document.getElementById('edit-title-input');

// Toggle between h1 and input for title editing
kanbanTitle.addEventListener('click', function () {
    kanbanTitle.style.display = 'none';
    editTitleInput.style.display = 'inline-block';
    editTitleInput.value = kanbanTitle.textContent.trim();
    editTitleInput.focus();
});

editTitleInput.addEventListener('blur', function () {
    const newTitle = editTitleInput.value.trim();
    if (newTitle === '') {
        editTitleInput.value = kanbanTitle.textContent.trim(); // Revert to previous title if empty
    } else {
        kanbanTitle.textContent = newTitle;
    }
    kanbanTitle.style.display = 'inline-block';
    editTitleInput.style.display = 'none';
});
