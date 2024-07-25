import styles from './TaskModal.module.css';

import { createSignal, Index, Show } from "solid-js";
import type { Accessor, Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";
import { LabelMenu } from './modal/LabelMenu';

type TaskModalProps = {
    getKanbanBoard: Accessor<KanbanMarkdown.KanbanBoard>;
    setKanbanBoard: Setter<KanbanMarkdown.KanbanBoard>;
    setTaskModalState: Setter<boolean>;
    getSelectedList: Accessor<KanbanMarkdown.KanbanList | undefined>;
    setSelectedTask: Setter<KanbanMarkdown.KanbanTask | undefined>;
    getSelectedTask: Accessor<KanbanMarkdown.KanbanTask | undefined>;
};

export const TaskModal: Component<TaskModalProps> = (props) => {
    const {
        getKanbanBoard,
        setKanbanBoard,
        setTaskModalState,
        getSelectedList,
        setSelectedTask,
        getSelectedTask
    } = props;

    const [getLabelMenuState, setLabelMenuState] = createSignal<boolean>(false);
    const [getAttachmentMenuState, setAttachmentMenuState] = createSignal<boolean>(false);

    const [getLabelMenuReference, setLabelMenuReference] = createSignal<HTMLDivElement>();
    let attachment_menu_reference: HTMLDivElement;

    function positionMenu(menu, button) {
        const buttonRect = button.getBoundingClientRect();
        const buttonHeight = button.offsetHeight;
        const buttonOffsetTop = buttonRect.top + window.scrollY;
        const buttonOffsetLeft = buttonRect.left + window.scrollX;

        menu.style.position = 'absolute';
        menu.style.top = `${buttonOffsetTop + buttonHeight}px`;
        menu.style.left = `${buttonOffsetLeft}px`;
        menu.style.display = 'flex';
        menu.style.flexDirection = 'column'; // Added to make the menu vertical
    }

    let modal_edit_card_title_reference: HTMLTextAreaElement;
    let modal_edit_card_description: HTMLTextAreaElement;

    return (
        <div class={styles.modal}>
            <div id={styles.modal_content}>
                <span id={styles.modal_close} onClick={() => {
                    setTaskModalState(false);
                }}>&times;</span>
                <div id={styles.modal_main}>
                    <div id={styles.modal_header}>
                        <h2>Edit Card</h2>
                        <Show when={getSelectedTask().labels && getSelectedTask().labels.length > 0}>
                            <div id={styles.modal_label_bar}>
                                <Index each={getSelectedTask().labels} >
                                    {(kanban_label, index) => (
                                        <div class={styles.modal_label} style={`background-color: ${kanban_label().color}`}
                                            onClick={() => {
                                                const selectedList = getSelectedList();
                                                const selectedTask = getSelectedTask();
                                                // @ts-ignore
                                                vscode.postMessage({
                                                    commands: [
                                                        {
                                                            action: 'delete',
                                                            path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].labels["${encodeURI(kanban_label().name)}"]`
                                                        }
                                                    ]
                                                });
                                                setSelectedTask(selectedTask => {
                                                    selectedTask.labels = selectedTask.labels.filter(label => label.name !== kanban_label().name);
                                                    return selectedTask;
                                                })
                                            }}>
                                            {kanban_label().name}
                                        </div>
                                    )}
                                </Index>
                            </div>
                        </Show>
                    </div>
                    <div id={styles.modal_body}>
                        <label for="modal_edit_card_title">Title</label>
                        <textarea ref={modal_edit_card_title_reference} id={styles.modal_edit_card_title} placeholder="Enter card title">{getSelectedTask().name}</textarea>
                        <label for="modal_edit_card_description">Description</label>
                        <textarea ref={modal_edit_card_description} id={styles.modal_edit_card_description} placeholder="Enter card description">{getSelectedTask().description}</textarea>
                    </div>
                    <div id={styles.modal_footer}>
                        <button id={styles.modal_save_card} onClick={() => {
                            const selectedList = getSelectedList();
                            const selectedTask = getSelectedTask();
                            // Improve this
                            // @ts-ignore
                            vscode.postMessage({
                                commands: [
                                    {
                                        action: 'update',
                                        path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].name`,
                                        value: selectedTask.name
                                    },
                                    {
                                        action: 'update',
                                        path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].description`,
                                        value: selectedTask.description
                                    },
                                    {
                                        action: 'update',
                                        path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].checked`,
                                        value: selectedTask.checked
                                    },
                                ]
                            });
                            setSelectedTask(selectedTask => {
                                selectedTask.name = modal_edit_card_title_reference.value;
                                selectedTask.description = modal_edit_card_description.value;
                                return selectedTask;
                            });
                            setTaskModalState(false);
                        }}>Save</button>
                    </div>
                </div>
                <div id={styles.modal_sidebar}>
                    <button id={styles.modal_label_button}
                        onClick={(event) => {
                            event.stopPropagation();
                            setLabelMenuState(b => !b);
                            if (getLabelMenuState()) {
                                setAttachmentMenuState(false);
                                positionMenu(getLabelMenuReference(), event.target);
                            }
                        }}>Labels</button>
                    <button id={styles.modal_attachment_button}
                        onClick={(event) => {
                            event.stopPropagation();
                            setAttachmentMenuState(b => !b);
                            if (getAttachmentMenuState()) {
                                setLabelMenuState(false);
                                positionMenu(attachment_menu_reference, event.target);
                            }
                        }}>Attachments</button>
                </div>
            </div>
            <Show when={getLabelMenuState()}>
                <LabelMenu
                    setLabelMenuReference={setLabelMenuReference}
                    getKanbanBoard={getKanbanBoard}
                    setKanbanBoard={setKanbanBoard}
                    getSelectedList={getSelectedList}
                    setSelectedTask={setSelectedTask}
                    getSelectedTask={getSelectedTask}
                />
            </Show>
            <Show when={getAttachmentMenuState()}>
                <div ref={attachment_menu_reference} class={styles.menu}>Attachment menu content</div>
            </Show>
        </div>
    );
}
