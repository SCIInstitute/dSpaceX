import React from 'react';

/**
 * Provides container for resizable panels
 * which can be populated with any view.
 */
class ResizeablePanels extends React.Component {
  constructor() {
    super();

    // TODO What state do we need
    this.state = {
      isDragging: false,
    };

    this.startResize = this.startResize.bind(this);
    this.resizePanel = this.resizePanel.bind(this);
    this.stopResize = this.stopResize.bind(this);
  }

  componentDidMount() {
    // TODO start by finding the element this way and then play with changing it to a ref
    const element = this.refs.resizeablePanel;
    element.addEventListener('mousemove', this.resizePanel);
    element.addEventListener('mouseup', this.stopResize);
    element.addEventListener('mouseleave', this.stopResize);
  }

  startResize(event, index) {
    this.setState({
      isDragging: true,
      currentPanel: index,
      initialPos: event.clientX,
    });
  }

  resizePanel(event) {
    if (this.state.isDragging) {
      const delta = event.clientX - this.state.initialPos;
      this.setState({
        delta: delta,
      });
    }
  }

  stopResize() {
    if (this.state.isDragging) {
      const { panels, currentPanel, delta } = this.state;
      let newPanels = [...panels];
      newPanels[currentPanel] = (panels[currentPanel] || 0) - delta;
      newPanels[currentPanel - 1] = (panels[currentPanel - 1] || 0) + delta;
      this.setState({
        isDragging: false,
        panels: newPanels,
        delta: 0,
        currentPanel: null,
      });
    }
  }

  render() {
    const rest = this.props.children.slice(1); // Grabbing last panels
    return (
      <div>

      </div>
    )
  }
}
export default ResizeablePanels;
