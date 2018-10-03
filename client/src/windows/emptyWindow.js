import React from 'react';


/**
 * A Window Component that renders nothing.
 */
class EmptyWindow extends React.Component {
  /**
   * EmptyWindow constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    return (
      <div style={{
        borderColor: '#ddddff',
        borderWidth: '2px',
        borderStyle: 'dashed',
        display: 'flex',
        justifyContent: 'center',
        alignItems: 'center',
      }}>
        <span style={{
          fontSize: 72,
          color: '#ddddff',
        }}>
          {this.props.id + 1}
        </span>
      </div>
    );
  }
}

export default EmptyWindow;
