import React from 'react';

class Workspace extends React.Component {
  constructor(props) {
    super(props)
  }

  render() {
    return <div className={this.props.className}>{this.props.children}</div>
  }
}

export default Workspace;
