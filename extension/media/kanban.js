document.getElementById('add-list').addEventListener('click', function() {
    const board = document.getElementById('board');
    
    const newList = document.createElement('div');
    newList.className = 'list';
    
    const listTitle = document.createElement('input');
    listTitle.className = 'list-title';
    listTitle.placeholder = 'Enter list title';
    
    const cards = document.createElement('div');
    cards.className = 'cards';
    
    const addCardButton = document.createElement('button');
    addCardButton.className = 'add-card';
    addCardButton.innerText = 'Add another card';
    addCardButton.addEventListener('click', function() {
        const card = document.createElement('div');
        card.className = 'card';
        
        const cardTitle = document.createElement('input');
        cardTitle.className = 'card-title';
        cardTitle.placeholder = 'Enter card title';
        
        card.appendChild(cardTitle);
        cards.appendChild(card);
    });
    
    newList.appendChild(listTitle);
    newList.appendChild(cards);
    newList.appendChild(addCardButton);
    
    board.insertBefore(newList, document.getElementById('add-list'));
});

document.querySelectorAll('.add-card').forEach(button => {
    button.addEventListener('click', function() {
        const card = document.createElement('div');
        card.className = 'card';
        
        const cardTitle = document.createElement('input');
        cardTitle.className = 'card-title';
        cardTitle.placeholder = 'Enter card title';
        
        card.appendChild(cardTitle);
        this.previousElementSibling.appendChild(card);
    });
});
