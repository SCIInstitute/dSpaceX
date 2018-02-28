import React from 'react';

class Application extends React.Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    console.log('Initializing Application...');
  }

  componentDidMount() {
    console.log('Application Ready.');
  }

  render() {
    return (
      <div>
        Insert Application Here.
      </div>
    );
  }
}

export default Application;