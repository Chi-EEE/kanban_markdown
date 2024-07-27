import styles from './TaskModal.module.css';

import { createSignal, For, Show } from "solid-js";
import type { Accessor, Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";
import { LabelMenu } from './modal/LabelMenu';
import { SetStoreFunction } from 'solid-js/store';

type TaskModalProps = {
    state: KanbanMarkdown.State;
    setState: SetStoreFunction<KanbanMarkdown.State>;

    setTaskModalState: Setter<boolean>;
};

export const TaskModal: Component<TaskModalProps> = (props) => {
    const {
        state,
        setState,
        setTaskModalState,
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
        menu.style.flexDirection = 'column';
    }

    let modal_edit_card_title_reference: HTMLTextAreaElement;
    let modal_edit_card_description: HTMLTextAreaElement;

    const updateSelectedTaskLabels = (labelName: string) => {
        const selectedList = state.selectedList;
        const selectedTask = state.selectedTask;
        if (selectedTask) {
            // @ts-ignore
            vscode.postMessage({
                commands: [
                    {
                        action: 'delete',
                        path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].labels["${encodeURI(labelName)}"]`
                    }
                ]
            });
            const updatedLabels = selectedTask.labels.filter(label => label.name !== labelName);
            setState("selectedList", "tasks", (task, index) => task.name === selectedTask.name, {
                labels: updatedLabels
            });
        }
    };

    const saveTask = () => {
        const selectedList = state.selectedList;
        const selectedTask = state.selectedTask;
        if (selectedTask && selectedList) {
            // @ts-ignore
            vscode.postMessage({
                commands: [
                    {
                        action: 'update',
                        path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].name`,
                        value: modal_edit_card_title_reference.value
                    },
                    {
                        action: 'update',
                        path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].description`,
                        value: modal_edit_card_description.value
                    },
                ]
            });
            setState("selectedList", "tasks", (task, index) => task.name === selectedTask.name, {
                name: modal_edit_card_title_reference.value,
                description: modal_edit_card_description.value
            });
            setTaskModalState(false);
        }
    };

    return (
        <div class={styles.modal}>
            <div id={styles.modal_content}>
                <span id={styles.modal_close} onClick={() => {
                    setTaskModalState(false);
                }}>&times;</span>
                <div id={styles.modal_main}>
                    <div id={styles.modal_header}>
                        <h2>Edit Card</h2>
                        <Show when={state.selectedTask.labels && state.selectedTask.labels.length > 0}>
                            <div id={styles.modal_label_bar}>
                                <For each={state.selectedTask.labels}>
                                    {(kanban_label) => (
                                        <div class={styles.modal_label} style={`background-color: ${kanban_label.color}`}
                                            onClick={() => updateSelectedTaskLabels(kanban_label.name)}>
                                            {kanban_label.name}
                                        </div>
                                    )}
                                </For>
                            </div>
                        </Show>
                    </div>
                    <div id={styles.modal_body}>
                        <label for="modal_edit_card_title">Title</label>
                        <textarea ref={modal_edit_card_title_reference} id={styles.modal_edit_card_title} placeholder="Enter card title">{state.selectedTask.name}</textarea>
                        <label for="modal_edit_card_description">Description</label>
                        <textarea ref={modal_edit_card_description} id={styles.modal_edit_card_description} placeholder="Enter card description">{state.selectedTask.description}</textarea>
                    </div>
                    <div id={styles.modal_footer}>
                        <button id={styles.modal_save_card} onClick={saveTask}>Save</button>
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
                    state={state}
                    setState={setState}
                />
            </Show>
            <Show when={getAttachmentMenuState()}>
                <div ref={attachment_menu_reference} class={styles.menu}>Attachment menu content</div>
            </Show>
        </div>
    );
}