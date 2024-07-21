export function applyAutoResize(element: HTMLElement) {
    element.style.height = '0';
    element.style.height = element.scrollHeight + 'px';
}