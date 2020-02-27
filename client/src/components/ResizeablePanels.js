import React from 'react';

// const styles = {
//   Container: {
//     display: 'flex',
//     flexWrap: 'wrap',
//     // alignItems:'stretch',
//   },
//   VerticalPanel: {
//     width: '49%',
//     backgroundColor: 'white',
//   },
//   VerticalDivider: {
//     width: '2px',
//     backgroundColor: 'grey',
//   },
//   HorizontalDivider: {
//     // width: '100%',
//     height: '2px',
//     backgroundColor: 'grey',
//   },
//   HorizontalPanel: {
//     // width: '100%',
//     backgroundColor: 'white',
//   },
// };
const styles = {
  Container: {
    display: 'flex',
    flexDirection: 'column'
  }
}

export default (props) => {
  return <div></div>;
  /**
   * What i want to do is create an outer div that then contains the div for the two upper panels
   * and the div for the lower larger panel and manage these all with flexbox. The outer div will be aligned on teh column
   * and then the divs it containt will be aligned so I get the result I need.
   */
};
